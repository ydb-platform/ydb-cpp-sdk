#pragma once

#include <tests/slo_workloads/utils/statistics.h>
#include <tests/slo_workloads/utils/utils.h>

#include <ydb-cpp-sdk/client/topic/client.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <stop_token>
#include <string>

struct TTopicOptions {
  explicit TTopicOptions(const TDatabaseOptions &databaseOptions)
      : DatabaseOptions(databaseOptions) {}

  TDatabaseOptions DatabaseOptions;
  std::uint32_t SecondsToRun = 10;
  std::uint32_t PartitionCount = 10;
  std::string ConsumerName = "slo-consumer";
  std::uint32_t ReaderCount = 5;
  std::string ProducerIdPrefix = "producer";
  std::uint32_t WriterCount = 20;
  bool DontPushMetrics = false;
  std::string MetricsPushUrl = "http://localhost:9090/api/v1/otlp/v1/metrics";
  std::string TopicPath;
};

class TTopicRunContext {
public:
  TTopicRunContext(const TTopicOptions &options, std::stop_source stopSource);
  ~TTopicRunContext();

  const TTopicOptions &GetOptions() const;
  void Fail(std::string message);
  bool Failed() const;

  void Start();
  void Finish();

  std::shared_ptr<TStatUnit> StartRead();
  void FinishRead(const std::shared_ptr<TStatUnit> &stat, bool success);
  void CancelRead(const std::shared_ptr<TStatUnit> &stat);

  std::shared_ptr<TStatUnit> StartWrite();
  void FinishWrite(const std::shared_ptr<TStatUnit> &stat, bool success);
  void CancelWrite(const std::shared_ptr<TStatUnit> &stat);

  bool
  ProcessDataEvent(NYdb::NTopic::TReadSessionEvent::TDataReceivedEvent &event);

private:
  struct TImpl;
  std::unique_ptr<TImpl> Impl_;
};

bool ParseTopicOptions(int argc, char **argv, TTopicOptions &options);

NYdb::NTopic::TReadSessionSettings
MakeTopicReadSettings(const TTopicOptions &options);
NYdb::NTopic::TWriteSessionSettings
MakeTopicWriteSettings(const TTopicOptions &options, std::uint32_t writerIndex);

bool HandleTopicReadEvent(NYdb::NTopic::TReadSessionEvent::TEvent &event,
                          TTopicRunContext &context);

bool HandleTopicWriteEvent(
    NYdb::NTopic::TWriteSessionEvent::TEvent &event,
    std::optional<NYdb::NTopic::TContinuationToken> &token,
    std::optional<std::uint64_t> expectedAck, bool &acked,
    TTopicRunContext &context);

int DoCreate(TDatabaseOptions &dbOptions, int argc, char **argv);
int DoRun(TDatabaseOptions &dbOptions, int argc, char **argv);
int DoCleanup(TDatabaseOptions &dbOptions, int argc);
