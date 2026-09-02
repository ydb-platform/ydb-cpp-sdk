#include "topic.h"

#include <tests/slo_workloads/utils/metrics.h>

#include <library/cpp/getopt/last_getopt.h>

#include <util/stream/output.h>
#include <util/string/builder.h>
#include <util/string/cast.h>
#include <util/system/env.h>

#include <atomic>
#include <cctype>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace NLastGetopt;
using namespace NYdb;
using namespace NYdb::NTopic;

namespace {

TStatus MakeStatus(bool success) {
  return TStatus(success ? EStatus::SUCCESS : EStatus::CLIENT_INTERNAL_ERROR,
                 NIssue::TIssues());
}

std::string SanitizePathPart(std::string value) {
  for (char &c : value) {
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_') {
      c = '-';
    }
  }
  return value;
}

std::string MakeTopicPath(const TDatabaseOptions &dbOptions) {
  std::string workload = GetEnv("WORKLOAD_NAME");
  std::string ref = GetEnv("WORKLOAD_REF");
  if (workload.empty()) {
    workload = "cpp-topic";
  }
  if (ref.empty()) {
    ref = "current";
  }
  return JoinPath(dbOptions.Prefix, SanitizePathPart(workload) + "-" +
                                        SanitizePathPart(ref) + "-topic");
}

using TTopicRetryPolicy = NYdb::NTopic::IRetryPolicy;

class TCountingRetryPolicy final : public TTopicRetryPolicy {
private:
  class TState final : public TTopicRetryPolicy::IRetryState {
  public:
    TState(TTopicRetryPolicy::IRetryState::TPtr state,
           TTopicRetryCallback retryCallback)
        : State_(std::move(state)), RetryCallback_(std::move(retryCallback)) {}

    TMaybe<TDuration> GetNextRetryDelay(EStatus status) override {
      auto delay = State_->GetNextRetryDelay(status);
      if (delay) {
        RetryCallback_();
      }
      return delay;
    }

  private:
    TTopicRetryPolicy::IRetryState::TPtr State_;
    TTopicRetryCallback RetryCallback_;
  };

public:
  explicit TCountingRetryPolicy(TTopicRetryCallback retryCallback)
      : Policy_(TTopicRetryPolicy::GetDefaultPolicy()),
        RetryCallback_(std::move(retryCallback)) {}

  TTopicRetryPolicy::IRetryState::TPtr CreateRetryState() const override {
    return std::make_unique<TState>(Policy_->CreateRetryState(),
                                    RetryCallback_);
  }

private:
  TTopicRetryPolicy::TPtr Policy_;
  TTopicRetryCallback RetryCallback_;
};

TTopicRetryPolicy::TPtr
MakeCountingRetryPolicy(TTopicRetryCallback retryCallback) {
  return std::make_shared<TCountingRetryPolicy>(std::move(retryCallback));
}

} // namespace

struct TTopicRunContext::TImpl {
  TImpl(const TTopicOptions &options, std::stop_source stopSource)
      : Options(options), StopSource(std::move(stopSource)),
        ReadStats(options.DontPushMetrics
                      ? std::nullopt
                      : std::make_optional(options.MetricsPushUrl),
                  "read"),
        WriteStats(options.DontPushMetrics
                       ? std::nullopt
                       : std::make_optional(options.MetricsPushUrl),
                   "write") {}

  TTopicOptions Options;
  std::stop_source StopSource;
  TStat ReadStats;
  TStat WriteStats;

  std::mutex DeliveryMutex;
  std::unordered_map<std::uint64_t, std::uint64_t> LastOffsets;

  std::atomic<bool> Failure{false};
  std::mutex FailureMutex;
  std::string FailureMessage;
};

TTopicRunContext::TTopicRunContext(const TTopicOptions &options,
                                   std::stop_source stopSource)
    : Impl_(std::make_unique<TImpl>(options, std::move(stopSource))) {}

TTopicRunContext::~TTopicRunContext() = default;

const TTopicOptions &TTopicRunContext::GetOptions() const {
  return Impl_->Options;
}

void TTopicRunContext::Fail(std::string message) {
  if (!Impl_->Failure.exchange(true)) {
    {
      std::lock_guard lock(Impl_->FailureMutex);
      Impl_->FailureMessage = std::move(message);
      Cerr << "Topic workload failed: " << Impl_->FailureMessage << Endl;
    }
    Impl_->StopSource.request_stop();
  }
}

bool TTopicRunContext::Failed() const { return Impl_->Failure.load(); }

void TTopicRunContext::Start() {
  Impl_->ReadStats.Start();
  Impl_->WriteStats.Start();
}

void TTopicRunContext::Finish() {
  Impl_->ReadStats.Finish();
  Impl_->WriteStats.Finish();
  if (!Impl_->WriteStats.ForceFlushMetrics()) {
    Fail("failed to flush topic metrics");
  }

  TStringBuilder report;
  report << Endl << "======- Topic read report -======" << Endl;
  Impl_->ReadStats.PrintStatistics(report);
  report << Endl << "======- Topic write report -=====" << Endl;
  Impl_->WriteStats.PrintStatistics(report);
  Cout << report;
}

std::shared_ptr<TStatUnit> TTopicRunContext::StartRead() {
  return Impl_->ReadStats.StartRequest();
}

void TTopicRunContext::FinishRead(const std::shared_ptr<TStatUnit> &stat,
                                  bool success, TInstant end) {
  Impl_->ReadStats.FinishRequest(stat, MakeStatus(success), end);
}

void TTopicRunContext::RecordReadRetry() { Impl_->ReadStats.RecordRetry(); }

std::shared_ptr<TStatUnit> TTopicRunContext::StartWrite() {
  return Impl_->WriteStats.StartRequest();
}

void TTopicRunContext::FinishWrite(const std::shared_ptr<TStatUnit> &stat,
                                   bool success) {
  Impl_->WriteStats.FinishRequest(stat, MakeStatus(success));
}

void TTopicRunContext::CancelWrite(const std::shared_ptr<TStatUnit> &stat) {
  Impl_->WriteStats.CancelRequest(stat);
}

void TTopicRunContext::RecordWriteRetry() { Impl_->WriteStats.RecordRetry(); }

bool TTopicRunContext::ProcessDataEvent(
    TReadSessionEvent::TDataReceivedEvent &event) {
  struct TDelivery {
    std::uint64_t Offset;
    TInstant CreatedAt;
    TInstant ReceivedAt;
  };

  std::vector<TDelivery> deliveries;
  std::optional<std::string> error;

  try {
    for (const auto &message : event.GetMessages()) {
      if (message.GetData() != "message") {
        error = "message has unexpected payload";
        break;
      }
      const TInstant createdAt = message.GetCreateTime();
      const TInstant now = TInstant::Now();
      if (createdAt == TInstant::Zero()) {
        error = "message has no creation timestamp";
        break;
      }
      if (createdAt > now) {
        error = "message creation timestamp is in the future";
        break;
      }
      deliveries.push_back({message.GetOffset(), createdAt, now});
    }
  } catch (const std::exception &e) {
    error = TStringBuilder() << "message processing failed: " << e.what();
  }

  if (error) {
    Fail(std::move(*error));
    return false;
  }

  std::vector<std::pair<TInstant, TInstant>> newDeliveries;
  newDeliveries.reserve(deliveries.size());
  {
    std::lock_guard lock(Impl_->DeliveryMutex);
    const auto partitionId = event.GetPartitionSession()->GetPartitionId();
    auto last = Impl_->LastOffsets.find(partitionId);
    for (const auto &delivery : deliveries) {
      if (last != Impl_->LastOffsets.end() && delivery.Offset <= last->second) {
        continue;
      }
      if (last == Impl_->LastOffsets.end()) {
        last = Impl_->LastOffsets.emplace(partitionId, delivery.Offset).first;
      } else {
        last->second = delivery.Offset;
      }
      newDeliveries.emplace_back(delivery.CreatedAt, delivery.ReceivedAt);
    }
  }

  for (const auto &[createdAt, receivedAt] : newDeliveries) {
    auto stat = StartRead();
    stat->Start = createdAt;
    FinishRead(stat, true, receivedAt);
  }
  return true;
}

bool ParseTopicOptions(int argc, char **argv, TTopicOptions &options) {
  if (std::string duration = GetEnv("WORKLOAD_DURATION"); !duration.empty()) {
    try {
      options.SecondsToRun = FromString<std::uint32_t>(duration);
    } catch (const std::exception &e) {
      Cerr << "Invalid WORKLOAD_DURATION '" << duration << "': " << e.what()
           << Endl;
      return false;
    }
  }
  if (std::string url = GetEnv("OTEL_EXPORTER_OTLP_METRICS_ENDPOINT");
      !url.empty()) {
    options.MetricsPushUrl = std::move(url);
  }

  TOpts opts = TOpts::Default();
  opts.AddHelpOption('h');
  opts.AddLongOption("time", "Time to run (seconds)")
      .RequiredArgument("SECONDS")
      .DefaultValue(options.SecondsToRun)
      .StoreResult(&options.SecondsToRun);
  opts.AddLongOption("partition-count", "Topic partition count")
      .RequiredArgument("NUM")
      .DefaultValue(options.PartitionCount)
      .StoreResult(&options.PartitionCount);
  opts.AddLongOption("consumer", "Topic consumer name")
      .RequiredArgument("NAME")
      .DefaultValue(options.ConsumerName)
      .StoreResult(&options.ConsumerName);
  opts.AddLongOption("reader-count", "Topic reader count")
      .RequiredArgument("NUM")
      .DefaultValue(options.ReaderCount)
      .StoreResult(&options.ReaderCount);
  opts.AddLongOption("producer-id-prefix", "Producer id prefix")
      .RequiredArgument("PREFIX")
      .DefaultValue(options.ProducerIdPrefix)
      .StoreResult(&options.ProducerIdPrefix);
  opts.AddLongOption("writer-count", "Topic writer count")
      .RequiredArgument("NUM")
      .DefaultValue(options.WriterCount)
      .StoreResult(&options.WriterCount);
  opts.AddLongOption("dont-push", "Do not push metrics")
      .NoArgument()
      .SetFlag(&options.DontPushMetrics)
      .DefaultValue(options.DontPushMetrics);
  opts.AddLongOption("metrics-push-url", "OTLP metrics endpoint")
      .RequiredArgument("URL")
      .DefaultValue(options.MetricsPushUrl)
      .StoreResult(&options.MetricsPushUrl);
  opts.MutuallyExclusive("dont-push", "metrics-push-url");
  opts.SetFreeArgsNum(0);

  TOptsParseResult result(&opts, argc, argv);
  Y_UNUSED(result);

  options.TopicPath = MakeTopicPath(options.DatabaseOptions);
  if (!options.SecondsToRun || !options.PartitionCount ||
      !options.ReaderCount || !options.WriterCount) {
    Cerr << "Topic workload counts and duration must be greater than zero"
         << Endl;
    return false;
  }
  return true;
}

TReadSessionSettings MakeTopicReadSettings(const TTopicOptions &options,
                                           TTopicRetryCallback retryCallback) {
  return TReadSessionSettings()
      .ConsumerName(options.ConsumerName)
      .AppendTopics(options.TopicPath)
      .RetryPolicy(MakeCountingRetryPolicy(std::move(retryCallback)));
}

TWriteSessionSettings
MakeTopicWriteSettings(const TTopicOptions &options, std::uint32_t writerIndex,
                       TTopicRetryCallback retryCallback) {
  const std::string producerId =
      options.ProducerIdPrefix + "-" + ToString(writerIndex);
  return TWriteSessionSettings()
      .Path(options.TopicPath)
      .ProducerId(producerId)
      .MessageGroupId(producerId)
      .PartitionId(writerIndex % options.PartitionCount)
      .RetryPolicy(MakeCountingRetryPolicy(std::move(retryCallback)))
      .Codec(ECodec::RAW);
}

bool HandleTopicReadEvent(TReadSessionEvent::TEvent &event,
                          TTopicRunContext &context) {
  if (auto *data = std::get_if<TReadSessionEvent::TDataReceivedEvent>(&event)) {
    try {
      if (!context.ProcessDataEvent(*data)) {
        return false;
      }
      data->Commit();
    } catch (const std::exception &e) {
      context.Fail(TStringBuilder() << "read event failed: " << e.what());
      return false;
    }
  } else if (auto *start =
                 std::get_if<TReadSessionEvent::TStartPartitionSessionEvent>(
                     &event)) {
    start->Confirm();
  } else if (auto *stop =
                 std::get_if<TReadSessionEvent::TStopPartitionSessionEvent>(
                     &event)) {
    stop->Confirm();
  } else if (auto *end =
                 std::get_if<TReadSessionEvent::TEndPartitionSessionEvent>(
                     &event)) {
    end->Confirm();
  } else if (auto *closed = std::get_if<TSessionClosedEvent>(&event)) {
    context.Fail(TStringBuilder() << "read session closed unexpectedly: "
                                  << closed->GetIssues().ToString());
    return false;
  }
  return !context.Failed();
}

bool HandleTopicWriteEvent(TWriteSessionEvent::TEvent &event,
                           std::optional<TContinuationToken> &token,
                           bool &acked, TTopicRunContext &context) {
  if (auto *ready =
          std::get_if<TWriteSessionEvent::TReadyToAcceptEvent>(&event)) {
    if (token) {
      context.Fail("write session returned an extra continuation token");
      return false;
    }
    token.emplace(std::move(ready->ContinuationToken));
    return true;
  }

  if (auto *acks = std::get_if<TWriteSessionEvent::TAcksEvent>(&event)) {
    for (const auto &ack : acks->Acks) {
      if (ack.State != TWriteSessionEvent::TWriteAck::EES_WRITTEN &&
          ack.State != TWriteSessionEvent::TWriteAck::EES_ALREADY_WRITTEN) {
        context.Fail(TStringBuilder() << "write was not persisted, ack state "
                                      << static_cast<int>(ack.State));
        return false;
      }
      acked = true;
    }
    return true;
  }

  if (auto *closed = std::get_if<TSessionClosedEvent>(&event)) {
    context.Fail(TStringBuilder() << "write session closed unexpectedly: "
                                  << closed->GetIssues().ToString());
  } else {
    context.Fail("write session returned an unknown event");
  }
  return false;
}

int DoCreate(TDatabaseOptions &dbOptions, int argc, char **argv) {
  TTopicOptions options(dbOptions);
  if (!ParseTopicOptions(argc, argv, options)) {
    return EXIT_FAILURE;
  }

  TCreateTopicSettings settings;
  settings.PartitioningSettings(options.PartitionCount, options.PartitionCount);
  settings.BeginAddConsumer(options.ConsumerName).Important(true);

  TTopicClient client(dbOptions.Driver);
  const TStatus status =
      client.CreateTopic(options.TopicPath, settings).GetValueSync();
  if (!status.IsSuccess()) {
    Cerr << "CreateTopic failed: " << status << Endl;
    return EXIT_FAILURE;
  }
  Cout << "Topic created: " << options.TopicPath << Endl;
  return EXIT_SUCCESS;
}

int DoCleanup(TDatabaseOptions &dbOptions, int argc) {
  if (argc > 1) {
    Cerr << "Unexpected arguments after cleanup" << Endl;
    return EXIT_FAILURE;
  }

  TTopicOptions options(dbOptions);
  options.TopicPath = MakeTopicPath(dbOptions);
  TTopicClient client(dbOptions.Driver);
  TStatus status = client.DropTopic(options.TopicPath).GetValueSync();
  if (status.GetStatus() == EStatus::NOT_FOUND) {
    return EXIT_SUCCESS;
  }
  if (!status.IsSuccess()) {
    Cerr << "DropTopic failed: " << status << Endl;
    return EXIT_FAILURE;
  }
  Cout << "Topic dropped: " << options.TopicPath << Endl;
  return EXIT_SUCCESS;
}
