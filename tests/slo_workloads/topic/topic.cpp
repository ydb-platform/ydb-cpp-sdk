#include "topic.h"

#include <tests/slo_workloads/utils/metrics.h>

#include <library/cpp/getopt/last_getopt.h>

#include <util/stream/output.h>
#include <util/string/builder.h>
#include <util/string/cast.h>
#include <util/system/env.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <deque>
#include <mutex>
#include <unordered_map>

using namespace NLastGetopt;
using namespace NYdb;
using namespace NYdb::NTopic;

namespace {

TStatus SuccessStatus() {
    return TStatus(EStatus::SUCCESS, NIssue::TIssues());
}

TStatus ErrorStatus() {
    return TStatus(EStatus::CLIENT_INTERNAL_ERROR, NIssue::TIssues());
}

std::string SanitizePathPart(std::string value) {
    for (char& c : value) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_') {
            c = '-';
        }
    }
    return value;
}

std::string MakeTopicPath(const TDatabaseOptions& dbOptions) {
    std::string workload = GetEnv("WORKLOAD_NAME");
    std::string ref = GetEnv("WORKLOAD_REF");
    if (workload.empty()) {
        workload = "cpp-topic";
    }
    if (ref.empty()) {
        ref = "current";
    }
    return JoinPath(
        dbOptions.Prefix,
        SanitizePathPart(workload) + "-" + SanitizePathPart(ref) + "-topic");
}

std::optional<std::string> MessageError(
    const TReadSessionEvent::TDataReceivedEvent::TMessage& message,
    std::unordered_map<std::string, std::uint64_t>& producerSeqNos,
    std::unordered_map<std::string, std::uint64_t>& streamSeqNos,
    const std::unordered_map<std::uint64_t, std::uint64_t>& committedOffsets)
{
    const std::uint64_t seqNo = message.GetSeqNo();
    const std::string expected = ToString(seqNo);
    if (message.GetData() != expected) {
        return TStringBuilder() << "payload mismatch for seq_no " << seqNo;
    }

    const std::string& producerId = message.GetProducerId();
    auto producer = producerSeqNos.find(producerId);
    if (producer == producerSeqNos.end()) {
        if (seqNo != 1) {
            return TStringBuilder() << "producer " << producerId
                << " starts at seq_no " << seqNo;
        }
        producerSeqNos.emplace(producerId, seqNo);
    } else if (seqNo == producer->second + 1) {
        producer->second = seqNo;
    } else if (seqNo > producer->second) {
        return TStringBuilder() << "producer " << producerId
            << " sequence gap after " << producer->second << ", got " << seqNo;
    }

    const auto partition = message.GetPartitionSession();
    const std::string streamKey = TStringBuilder()
        << producerId << ':' << partition->GetReadSessionId()
        << ':' << partition->GetPartitionSessionId();
    auto stream = streamSeqNos.find(streamKey);
    if (stream == streamSeqNos.end()) {
        streamSeqNos.emplace(streamKey, seqNo);
    } else if (seqNo == stream->second + 1) {
        stream->second = seqNo;
    } else {
        return TStringBuilder() << "stream sequence after " << stream->second
            << ", got " << seqNo;
    }

    const std::uint64_t partitionId = partition->GetPartitionId();
    if (auto it = committedOffsets.find(partitionId);
        it != committedOffsets.end() && message.GetOffset() < it->second)
    {
        return TStringBuilder() << "partition " << partitionId << " offset "
            << message.GetOffset() << " is below committed offset " << it->second;
    }

    return std::nullopt;
}

} // namespace

struct TTopicRunContext::TImpl {
    struct TPendingCommit {
        std::uint64_t EndOffset;
        TInstant Deadline;
        std::shared_ptr<TStatUnit> Stat;
    };

    explicit TImpl(const TTopicOptions& options)
        : Options(options)
        , Deadline(TInstant::Now() + TDuration::Seconds(options.SecondsToRun))
        , ReadStats(
            options.DontPushMetrics ? std::nullopt : std::make_optional(options.MetricsPushUrl),
            "read")
        , WriteStats(
            options.DontPushMetrics ? std::nullopt : std::make_optional(options.MetricsPushUrl),
            "write")
        , E2eMetrics(options.DontPushMetrics
            ? CreateNoopMetricsPusher()
            : CreateOtelMetricsPusher(options.MetricsPushUrl, "topic_e2e"))
    {}

    void ExpireCommits() {
        std::vector<std::shared_ptr<TStatUnit>> expired;
        const TInstant now = TInstant::Now();
        {
            std::lock_guard lock(Mutex);
            for (auto& [partition, commits] : PendingCommits) {
                Y_UNUSED(partition);
                while (!commits.empty() && commits.front().Deadline <= now) {
                    expired.push_back(std::move(commits.front().Stat));
                    commits.pop_front();
                }
            }
        }
        for (const auto& stat : expired) {
            ReadStats.FinishRequest(stat, TFinalStatus{});
        }
    }

    void AckCommit(std::uint64_t partitionId, std::uint64_t committedOffset) {
        std::vector<std::shared_ptr<TStatUnit>> completed;
        {
            std::lock_guard lock(Mutex);
            auto& current = CommittedOffsets[partitionId];
            current = std::max(current, committedOffset);

            auto& commits = PendingCommits[partitionId];
            while (!commits.empty() && commits.front().EndOffset <= committedOffset) {
                completed.push_back(std::move(commits.front().Stat));
                commits.pop_front();
            }
        }
        for (const auto& stat : completed) {
            ReadStats.FinishRequest(stat, SuccessStatus());
        }
    }

    void CancelCommits() {
        std::vector<std::shared_ptr<TStatUnit>> pending;
        {
            std::lock_guard lock(Mutex);
            for (auto& [partition, commits] : PendingCommits) {
                Y_UNUSED(partition);
                while (!commits.empty()) {
                    pending.push_back(std::move(commits.front().Stat));
                    commits.pop_front();
                }
            }
        }
        for (const auto& stat : pending) {
            ReadStats.CancelRequest(stat);
        }
    }

    TTopicOptions Options;
    TInstant Deadline;
    TStat ReadStats;
    TStat WriteStats;
    std::unique_ptr<IMetricsPusher> E2eMetrics;

    std::atomic<bool> Failure{false};
    mutable std::mutex FailureMutex;
    std::string FailureMessage;

    std::mutex Mutex;
    std::unordered_map<std::string, std::uint64_t> ProducerSeqNos;
    std::unordered_map<std::string, std::uint64_t> StreamSeqNos;
    std::unordered_map<std::uint64_t, std::uint64_t> CommittedOffsets;
    std::unordered_map<std::uint64_t, std::deque<TPendingCommit>> PendingCommits;
};

TTopicRunContext::TTopicRunContext(const TTopicOptions& options)
    : Impl_(std::make_unique<TImpl>(options))
{}

TTopicRunContext::~TTopicRunContext() = default;

const TTopicOptions& TTopicRunContext::GetOptions() const {
    return Impl_->Options;
}

bool TTopicRunContext::ShouldContinue() const {
    return !Impl_->Failure.load() && TInstant::Now() < Impl_->Deadline;
}

void TTopicRunContext::Fail(std::string message) {
    if (!Impl_->Failure.exchange(true)) {
        std::lock_guard lock(Impl_->FailureMutex);
        Impl_->FailureMessage = std::move(message);
        Cerr << "Topic workload failed: " << Impl_->FailureMessage << Endl;
    }
}

bool TTopicRunContext::Failed() const {
    return Impl_->Failure.load();
}

void TTopicRunContext::Start() {
    Impl_->ReadStats.Start();
    Impl_->WriteStats.Start();
}

void TTopicRunContext::Finish() {
    Impl_->CancelCommits();
    Impl_->ReadStats.Finish();
    Impl_->WriteStats.Finish();

    TStringBuilder report;
    report << Endl << "======- Topic read report -======" << Endl;
    Impl_->ReadStats.PrintStatistics(report);
    report << Endl << "======- Topic write report -=====" << Endl;
    Impl_->WriteStats.PrintStatistics(report);
    Cout << report;
}

std::shared_ptr<TStatUnit> TTopicRunContext::StartWrite() {
    return Impl_->WriteStats.StartRequest();
}

void TTopicRunContext::FinishWrite(
    const std::shared_ptr<TStatUnit>& stat,
    const TFinalStatus& status)
{
    Impl_->WriteStats.FinishRequest(stat, status);
}

void TTopicRunContext::CancelWrite(const std::shared_ptr<TStatUnit>& stat) {
    Impl_->WriteStats.CancelRequest(stat);
}

void TTopicRunContext::RunReader(std::unique_ptr<ITopicReadSession> session) {
    while (ShouldContinue()) {
        Impl_->ExpireCommits();

        auto delivery = Impl_->ReadStats.StartRequest();
        std::vector<TReadSessionEvent::TEvent> events;
        ETopicWaitResult waitResult;
        try {
            waitResult = session->WaitEvents(Impl_->Options.DeliveryTimeout, events);
        } catch (const std::exception& e) {
            Impl_->ReadStats.FinishRequest(delivery, ErrorStatus());
            Fail(TStringBuilder() << "reader wait failed: " << e.what());
            break;
        }

        if (waitResult == ETopicWaitResult::Timeout) {
            if (ShouldContinue()) {
                Impl_->ReadStats.FinishRequest(delivery, TFinalStatus{});
            } else {
                Impl_->ReadStats.CancelRequest(delivery);
            }
            continue;
        }
        if (waitResult == ETopicWaitResult::Cancelled) {
            Impl_->ReadStats.CancelRequest(delivery);
            if (ShouldContinue()) {
                Fail("reader wait was cancelled");
            }
            break;
        }
        Impl_->ReadStats.CancelRequest(delivery);

        for (auto& event : events) {
            if (auto* data = std::get_if<TReadSessionEvent::TDataReceivedEvent>(&event)) {
                std::uint64_t endOffset = 0;
                std::uint64_t partitionId = data->GetPartitionSession()->GetPartitionId();
                std::optional<std::string> error;

                for (const auto& message : data->GetMessages()) {
                    TDuration e2e;
                    {
                        std::lock_guard lock(Impl_->Mutex);
                        error = MessageError(
                            message,
                            Impl_->ProducerSeqNos,
                            Impl_->StreamSeqNos,
                            Impl_->CommittedOffsets);
                    }
                    if (error) {
                        break;
                    }

                    const TInstant createdAt = message.GetCreateTime();
                    const TInstant now = TInstant::Now();
                    if (createdAt == TInstant::Zero()) {
                        error = "message has no timestamp";
                        break;
                    }
                    if (createdAt > now) {
                        error = TStringBuilder() << "message timestamp is in the future";
                        break;
                    }
                    e2e = now - createdAt;
                    Impl_->E2eMetrics->PushRequestData({
                        .Delay = e2e,
                        .Status = EStatus::SUCCESS,
                        .RetryAttempts = 0,
                    });
                    endOffset = std::max(endOffset, message.GetOffset() + 1);
                }

                if (error) {
                    auto stat = Impl_->ReadStats.StartRequest();
                    Impl_->ReadStats.FinishRequest(stat, ErrorStatus());
                    Fail(*error);
                    break;
                }

                auto commit = Impl_->ReadStats.StartRequest();
                try {
                    data->Commit();
                    std::lock_guard lock(Impl_->Mutex);
                    Impl_->PendingCommits[partitionId].push_back({
                        .EndOffset = endOffset,
                        .Deadline = TInstant::Now() + Impl_->Options.CommitTimeout,
                        .Stat = std::move(commit),
                    });
                } catch (const std::exception& e) {
                    Impl_->ReadStats.FinishRequest(commit, ErrorStatus());
                    Fail(TStringBuilder() << "commit failed: " << e.what());
                    break;
                }
            } else if (auto* ack = std::get_if<TReadSessionEvent::TCommitOffsetAcknowledgementEvent>(&event)) {
                Impl_->AckCommit(
                    ack->GetPartitionSession()->GetPartitionId(),
                    ack->GetCommittedOffset());
            } else if (auto* start = std::get_if<TReadSessionEvent::TStartPartitionSessionEvent>(&event)) {
                start->Confirm();
            } else if (auto* stop = std::get_if<TReadSessionEvent::TStopPartitionSessionEvent>(&event)) {
                stop->Confirm();
            } else if (auto* end = std::get_if<TReadSessionEvent::TEndPartitionSessionEvent>(&event)) {
                end->Confirm();
            } else if (auto* closed = std::get_if<TSessionClosedEvent>(&event)) {
                if (!closed->IsSuccess()) {
                    auto stat = Impl_->ReadStats.StartRequest();
                    Impl_->ReadStats.FinishRequest(stat, ErrorStatus());
                    Fail(TStringBuilder() << "read session closed: " << closed->GetIssues().ToString());
                } else if (ShouldContinue()) {
                    auto stat = Impl_->ReadStats.StartRequest();
                    Impl_->ReadStats.FinishRequest(stat, ErrorStatus());
                    Fail("read session closed unexpectedly");
                }
                break;
            }
        }
    }

    try {
        session->Close(TDuration::Seconds(5));
    } catch (const std::exception& e) {
        Cerr << "Reader close failed: " << e.what() << Endl;
    }
}

bool ParseTopicOptions(int argc, char** argv, TTopicOptions& options) {
    if (std::string duration = GetEnv("WORKLOAD_DURATION"); !duration.empty()) {
        try {
            options.SecondsToRun = FromString<std::uint32_t>(duration);
        } catch (const std::exception& e) {
            Cerr << "Invalid WORKLOAD_DURATION '" << duration << "': " << e.what() << Endl;
            return false;
        }
    }
    if (std::string url = GetEnv("OTEL_EXPORTER_OTLP_METRICS_ENDPOINT"); !url.empty()) {
        options.MetricsPushUrl = std::move(url);
    }

    std::uint64_t writeTimeoutMs = options.WriteTimeout.MilliSeconds();
    std::uint64_t deliveryTimeoutMs = options.DeliveryTimeout.MilliSeconds();
    std::uint64_t commitTimeoutMs = options.CommitTimeout.MilliSeconds();

    TOpts opts = TOpts::Default();
    opts.AddHelpOption('h');
    opts.AddLongOption("time", "Time to run (seconds)").RequiredArgument("SECONDS")
        .DefaultValue(options.SecondsToRun).StoreResult(&options.SecondsToRun);
    opts.AddLongOption("write-rps", "Total messages submitted per second").RequiredArgument("NUM")
        .DefaultValue(options.WriteRps).StoreResult(&options.WriteRps);
    opts.AddLongOption("write-timeout", "Write acknowledgement timeout (ms)").RequiredArgument("MS")
        .DefaultValue(writeTimeoutMs).StoreResult(&writeTimeoutMs);
    opts.AddLongOption("delivery-timeout", "Message delivery timeout (ms)").RequiredArgument("MS")
        .DefaultValue(deliveryTimeoutMs).StoreResult(&deliveryTimeoutMs);
    opts.AddLongOption("commit-timeout", "Commit acknowledgement timeout (ms)").RequiredArgument("MS")
        .DefaultValue(commitTimeoutMs).StoreResult(&commitTimeoutMs);
    opts.AddLongOption("partition-count", "Topic partition count").RequiredArgument("NUM")
        .DefaultValue(options.PartitionCount).StoreResult(&options.PartitionCount);
    opts.AddLongOption("consumer", "Topic consumer name").RequiredArgument("NAME")
        .DefaultValue(options.ConsumerName).StoreResult(&options.ConsumerName);
    opts.AddLongOption("reader-count", "Topic reader count").RequiredArgument("NUM")
        .DefaultValue(options.ReaderCount).StoreResult(&options.ReaderCount);
    opts.AddLongOption("producer-id-prefix", "Producer id prefix").RequiredArgument("PREFIX")
        .DefaultValue(options.ProducerIdPrefix).StoreResult(&options.ProducerIdPrefix);
    opts.AddLongOption("writer-count", "Topic writer count").RequiredArgument("NUM")
        .DefaultValue(options.WriterCount).StoreResult(&options.WriterCount);
    opts.AddLongOption("dont-push", "Do not push metrics").NoArgument()
        .SetFlag(&options.DontPushMetrics).DefaultValue(options.DontPushMetrics);
    opts.AddLongOption("metrics-push-url", "OTLP metrics endpoint").RequiredArgument("URL")
        .DefaultValue(options.MetricsPushUrl).StoreResult(&options.MetricsPushUrl);
    opts.MutuallyExclusive("dont-push", "metrics-push-url");
    opts.SetFreeArgsNum(0);

    TOptsParseResult result(&opts, argc, argv);
    Y_UNUSED(result);

    options.WriteTimeout = TDuration::MilliSeconds(writeTimeoutMs);
    options.DeliveryTimeout = TDuration::MilliSeconds(deliveryTimeoutMs);
    options.CommitTimeout = TDuration::MilliSeconds(commitTimeoutMs);
    options.TopicPath = MakeTopicPath(options.DatabaseOptions);

    if (!options.SecondsToRun || !options.WriteRps || !options.PartitionCount ||
        !options.ReaderCount || !options.WriterCount || !writeTimeoutMs ||
        !deliveryTimeoutMs || !commitTimeoutMs)
    {
        Cerr << "Topic workload counts, RPS, duration, and timeouts must be greater than zero" << Endl;
        return false;
    }
    return true;
}

TReadSessionSettings MakeTopicReadSettings(const TTopicOptions& options) {
    return TReadSessionSettings()
        .ConsumerName(options.ConsumerName)
        .AppendTopics(options.TopicPath);
}

void RunTopicWriter(
    TTopicClient& client,
    std::uint32_t writerIndex,
    TTopicRunContext& context,
    const TTopicSleep& sleep)
{
    const auto& options = context.GetOptions();
    const std::uint32_t workerRps = options.WriteRps / options.WriterCount +
        (writerIndex < options.WriteRps % options.WriterCount ? 1 : 0);
    if (!workerRps) {
        while (context.ShouldContinue()) {
            sleep(TDuration::MilliSeconds(100));
        }
        return;
    }

    const std::string producerId = options.ProducerIdPrefix + "-" + ToString(writerIndex);
    TWriteSessionSettings settings;
    settings.Path(options.TopicPath)
        .ProducerId(producerId)
        .MessageGroupId(producerId)
        .Codec(ECodec::RAW);
    auto session = client.CreateWriteSession(settings);
    const TDuration period = TDuration::MicroSeconds(1'000'000 / workerRps);
    TInstant next = TInstant::Now();
    std::uint64_t seqNo = 1;

    while (context.ShouldContinue()) {
        const TInstant now = TInstant::Now();
        if (now < next) {
            sleep(next - now);
        }
        next = std::max(next + period, TInstant::Now());
        if (!context.ShouldContinue()) {
            break;
        }

        auto stat = context.StartWrite();
        std::optional<TContinuationToken> token;
        const TInstant tokenDeadline = TInstant::Now() + options.WriteTimeout;
        while (!token && context.ShouldContinue()) {
            const TInstant now = TInstant::Now();
            if (now >= tokenDeadline || !session->WaitEvent().Wait(tokenDeadline - now)) {
                break;
            }
            for (auto& event : session->GetEvents(false)) {
                if (auto* ready = std::get_if<TWriteSessionEvent::TReadyToAcceptEvent>(&event)) {
                    token.emplace(std::move(ready->ContinuationToken));
                } else if (auto* closed = std::get_if<TSessionClosedEvent>(&event)) {
                    context.Fail(TStringBuilder()
                        << "write session closed: " << closed->GetIssues().ToString());
                }
            }
        }

        if (!token) {
            if (context.Failed()) {
                context.FinishWrite(stat, ErrorStatus());
                break;
            }
            if (!context.ShouldContinue()) {
                context.CancelWrite(stat);
                break;
            }
            context.FinishWrite(stat, TFinalStatus{});
            continue;
        }

        try {
            const std::string payload = ToString(seqNo);
            TWriteMessage message(payload);
            message.SeqNo(seqNo);
            session->Write(std::move(*token), std::move(message));

            auto flush = session->Flush();
            if (!flush.Wait(options.WriteTimeout)) {
                context.FinishWrite(stat, TFinalStatus{});
            } else if (flush.GetValueSync()) {
                context.FinishWrite(stat, SuccessStatus());
            } else {
                context.FinishWrite(stat, ErrorStatus());
                context.Fail(TStringBuilder() << "write flush failed at seq_no " << seqNo);
            }
        } catch (const std::exception& e) {
            context.FinishWrite(stat, ErrorStatus());
            context.Fail(TStringBuilder() << "write failed: " << e.what());
        }
        ++seqNo;
    }

    session->Close(options.WriteTimeout);
}

int DoCreate(TDatabaseOptions& dbOptions, int argc, char** argv) {
    TTopicOptions options(dbOptions);
    if (!ParseTopicOptions(argc, argv, options)) {
        return EXIT_FAILURE;
    }

    TCreateTopicSettings settings;
    settings.PartitioningSettings(options.PartitionCount, options.PartitionCount);
    settings.BeginAddConsumer(options.ConsumerName).Important(true);

    TTopicClient client(dbOptions.Driver);
    const TStatus status = client.CreateTopic(options.TopicPath, settings).GetValueSync();
    if (!status.IsSuccess()) {
        Cerr << "CreateTopic failed: " << status << Endl;
        return EXIT_FAILURE;
    }
    Cout << "Topic created: " << options.TopicPath << Endl;
    return EXIT_SUCCESS;
}

int DoCleanup(TDatabaseOptions& dbOptions, int argc) {
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
