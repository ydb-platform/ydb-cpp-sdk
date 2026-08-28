#pragma once

#include <tests/slo_workloads/utils/statistics.h>
#include <tests/slo_workloads/utils/utils.h>

#include <ydb-cpp-sdk/client/topic/client.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct TTopicOptions {
    explicit TTopicOptions(const TDatabaseOptions& databaseOptions)
        : DatabaseOptions(databaseOptions)
    {}

    TDatabaseOptions DatabaseOptions;
    std::uint32_t SecondsToRun = 10;
    std::uint32_t WriteRps = 1000;
    TDuration WriteTimeout = TDuration::Seconds(5);
    TDuration DeliveryTimeout = TDuration::Seconds(5);
    TDuration CommitTimeout = TDuration::Seconds(5);
    std::uint32_t PartitionCount = 10;
    std::string ConsumerName = "slo-consumer";
    std::uint32_t ReaderCount = 5;
    std::string ProducerIdPrefix = "producer";
    std::uint32_t WriterCount = 20;
    bool DontPushMetrics = false;
    std::string MetricsPushUrl = "http://localhost:9090/api/v1/otlp/v1/metrics";
    std::string TopicPath;
};

enum class ETopicWaitResult {
    Events,
    Timeout,
    Cancelled,
};

class ITopicReadSession {
public:
    virtual ~ITopicReadSession() = default;

    virtual ETopicWaitResult WaitEvents(
        TDuration timeout,
        std::vector<NYdb::NTopic::TReadSessionEvent::TEvent>& events) = 0;
    virtual void Close(TDuration timeout) = 0;
};

class TTopicRunContext {
public:
    explicit TTopicRunContext(const TTopicOptions& options);
    ~TTopicRunContext();

    const TTopicOptions& GetOptions() const;
    bool ShouldContinue() const;
    void Fail(std::string message);
    bool Failed() const;

    void Start();
    void Finish();
    void RunReader(std::unique_ptr<ITopicReadSession> session);
    std::shared_ptr<TStatUnit> StartWrite();
    void FinishWrite(const std::shared_ptr<TStatUnit>& stat, const TFinalStatus& status);
    void CancelWrite(const std::shared_ptr<TStatUnit>& stat);

private:
    struct TImpl;
    std::unique_ptr<TImpl> Impl_;
};

using TTopicSleep = std::function<void(TDuration)>;

bool ParseTopicOptions(int argc, char** argv, TTopicOptions& options);
NYdb::NTopic::TReadSessionSettings MakeTopicReadSettings(const TTopicOptions& options);

void RunTopicWriter(
    NYdb::NTopic::TTopicClient& client,
    std::uint32_t writerIndex,
    TTopicRunContext& context,
    const TTopicSleep& sleep);

int DoCreate(TDatabaseOptions& dbOptions, int argc, char** argv);
int DoRun(TDatabaseOptions& dbOptions, int argc, char** argv);
int DoCleanup(TDatabaseOptions& dbOptions, int argc);
