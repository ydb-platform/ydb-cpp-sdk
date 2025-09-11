#pragma once

#include <ydb-cpp-sdk/client/types/status_codes.h>

#include <src/client/impl/internal/metrics/metrics.h>
#include <src/library/grpc/client/grpc_client_low.h>

#include <atomic>


namespace NYdb::inline V3 {
namespace NSdkStats {

// works only for case normal (foo_bar) underscore

std::string UnderscoreToUpperCamel(const std::string& in);

NMetrics::TBucketBounds ExponentialHistogram(std::uint32_t bucketsCount, double base, double scale);

template<typename TPointer>
class TAtomicPointer {
public:
    TAtomicPointer(std::shared_ptr<TPointer> pointer = nullptr) {
        Set(pointer);
    }

    TAtomicPointer(const TAtomicPointer& other) {
        Set(other.Get());
    }

    TAtomicPointer& operator=(const TAtomicPointer& other) {
        Set(other.Get());
        return *this;
    }

    std::shared_ptr<TPointer> Get() const {
        return Pointer_.load();
    }

    void Set(std::shared_ptr<TPointer> pointer) {
        Pointer_.store(pointer);
    }

private:
    std::atomic<std::shared_ptr<TPointer>> Pointer_;
};

template<typename TPointer>
class TAtomicCounter : public TAtomicPointer<TPointer> {
public:
    void Add(std::uint64_t value) {
        if (auto counter = this->Get()) {
            counter->Add(value);
        }
    }

    void Inc() {
        if (auto counter = this->Get()) {
            counter->Add(1);
        }
    }

    void Dec() {
        if (auto counter = this->Get()) {
            counter->Add(-1);
        }
    }

    void SetValue(std::uint64_t value) {
        if (auto counter = this->Get()) {
            counter->Set(value);
        }
    }
};

template<typename TCounter>
class TFastLocalCounter {
public:
    TFastLocalCounter(TAtomicCounter<TCounter>& counter)
        : Counter(counter)
        , Value(0)
    {}

    ~TFastLocalCounter() {
        Counter.Add(Value);
    }

    TFastLocalCounter<TCounter>& operator++ () {
        ++Value;
        return *this;
    }

    TAtomicCounter<TCounter>& Counter;
    std::uint64_t Value;
};

template<typename TPointer>
class TAtomicHistogram: public TAtomicPointer<TPointer> {
public:
    void Record(std::int64_t value) {
        if (auto histogram = this->Get()) {
            histogram->Record(value);
        }
    }

    bool IsCollecting() {
        return this->Get() != nullptr;
    }
};

// Sessions count for all clients
// Every client has 3 TSessionCounter for active, in session pool, in settler sessions
// TSessionCounters in different clients with same role share one sensor
class TSessionCounter : public TAtomicPointer<NMetrics::IIntGauge> {
public:
    // Call with mutex
    void Apply(std::int64_t newValue) {
        if (auto gauge = this->Get()) {
            gauge->Add(newValue - oldValue);
            oldValue = newValue;
        }
    }

    ~TSessionCounter() {
        auto gauge = this->Get();
        if (gauge) {
            gauge->Add(-oldValue);
        }
    }

private:
    std::int64_t oldValue = 0;
};

struct TStatCollector {
public:
    struct TEndpointElectorStatCollector {
        TEndpointElectorStatCollector(std::shared_ptr<NMetrics::IIntGauge> endpointCount = nullptr,
                                      std::shared_ptr<NMetrics::IIntGauge> pessimizationRatio = nullptr,
                                      std::shared_ptr<NMetrics::IIntGauge> activeEndpoints = nullptr)
            : EndpointCount(endpointCount)
            , PessimizationRatio(pessimizationRatio)
            , EndpointActive(activeEndpoints)
        {}

        std::shared_ptr<NMetrics::IIntGauge> EndpointCount;
        std::shared_ptr<NMetrics::IIntGauge> PessimizationRatio;
        std::shared_ptr<NMetrics::IIntGauge> EndpointActive;
    };

    struct TSessionPoolStatCollector {
        TSessionPoolStatCollector(std::shared_ptr<NMetrics::IIntGauge> activeSessions = nullptr,
                                  std::shared_ptr<NMetrics::IIntGauge> inPoolSessions = nullptr,
                                  std::shared_ptr<NMetrics::IRate> fakeSessions = nullptr,
                                  std::shared_ptr<NMetrics::IIntGauge> waiters = nullptr)
            : ActiveSessions(activeSessions)
            , InPoolSessions(inPoolSessions)
            , FakeSessions(fakeSessions)
            , Waiters(waiters)
        {}

        std::shared_ptr<NMetrics::IIntGauge> ActiveSessions;
        std::shared_ptr<NMetrics::IIntGauge> InPoolSessions;
        std::shared_ptr<NMetrics::IRate> FakeSessions;
        std::shared_ptr<NMetrics::IIntGauge> Waiters;
    };

    struct TClientRetryOperationStatCollector {
        TClientRetryOperationStatCollector()
            : MetricRegistry_()
            , Database_()
        {}

        TClientRetryOperationStatCollector(std::shared_ptr<NMetrics::IMetricsProvider> registry,
                                           const std::string& database,
                                           const std::string& clientType)
            : MetricRegistry_(registry)
            , Database_(database)
            , ClientType_(clientType)
        {}

        void IncSyncRetryOperation(const EStatus& status) {
            if (MetricRegistry_) {
                std::string statusName = TStringBuilder() << status;
                std::string sensor = TStringBuilder() << "RetryOperation/" << UnderscoreToUpperCamel(statusName);
                MetricRegistry_->Rate({ {"database", Database_}, {"ydb_client", ClientType_}, {"sensor", sensor} })->Add(1);
            }
        }

        void IncAsyncRetryOperation(const EStatus& status) {
            if (auto registry = MetricRegistry_) {
                std::string statusName = TStringBuilder() << status;
                std::string sensor = TStringBuilder() << "RetryOperation/" << UnderscoreToUpperCamel(statusName);
                MetricRegistry_->Rate({ {"database", Database_}, {"ydb_client", ClientType_}, {"sensor", sensor} })->Add(1);
            }
        }

    private:
        std::shared_ptr<NMetrics::IMetricsProvider> MetricRegistry_;
        std::string Database_;
        std::string ClientType_;
    };

    struct TClientStatCollector {
        TClientStatCollector(std::shared_ptr<NMetrics::IRate> cacheMiss = nullptr,
                             std::shared_ptr<NMetrics::IHistogram> querySize = nullptr,
                             std::shared_ptr<NMetrics::IHistogram> paramsSize = nullptr,
                             std::shared_ptr<NMetrics::IRate> sessionRemoved = nullptr,
                             std::shared_ptr<NMetrics::IRate> requestMigrated = nullptr,
                             TClientRetryOperationStatCollector retryOperationStatCollector = TClientRetryOperationStatCollector())
            : CacheMiss(cacheMiss)
            , QuerySize(querySize)
            , ParamsSize(paramsSize)
            , SessionRemovedDueBalancing(sessionRemoved)
            , RequestMigrated(requestMigrated)
            , RetryOperationStatCollector(retryOperationStatCollector)
        {}

        std::shared_ptr<NMetrics::IRate> CacheMiss;
        std::shared_ptr<NMetrics::IHistogram> QuerySize;
        std::shared_ptr<NMetrics::IHistogram> ParamsSize;
        std::shared_ptr<NMetrics::IRate> SessionRemovedDueBalancing;
        std::shared_ptr<NMetrics::IRate> RequestMigrated;

        TClientRetryOperationStatCollector RetryOperationStatCollector;
    };

    TStatCollector(const std::string& database, std::shared_ptr<NMetrics::IMetricsProvider> sensorsRegistry)
        : Database_(database)
        , DatabaseLabel_({"database", database})
    {
        if (sensorsRegistry) {
            SetMetricRegistry(sensorsRegistry);
        }
    }

    void SetMetricRegistry(std::shared_ptr<NMetrics::IMetricsProvider> sensorsRegistry) {
        Y_ABORT_UNLESS(sensorsRegistry, "TMetricRegistry is null in stats collector.");
        MetricRegistryPtr_.Set(sensorsRegistry);
        DiscoveryDuePessimization_.Set(sensorsRegistry->Rate({ DatabaseLabel_,      {"sensor", "Discovery/TooManyBadEndpoints"} }));
        DiscoveryDueExpiration_.Set(sensorsRegistry->Rate({ DatabaseLabel_,         {"sensor", "Discovery/Regular"} }));
        DiscoveryFailDueTransportError_.Set(sensorsRegistry->Rate({ DatabaseLabel_, {"sensor", "Discovery/FailedTransportError"} }));
        RequestFailDueQueueOverflow_.Set(sensorsRegistry->Rate({ DatabaseLabel_,    {"sensor", "Request/FailedDiscoveryQueueOverflow"} }));
        RequestFailDueNoEndpoint_.Set(sensorsRegistry->Rate({ DatabaseLabel_,       {"sensor", "Request/FailedNoEndpoint"} }));
        RequestFailDueTransportError_.Set(sensorsRegistry->Rate({ DatabaseLabel_,   {"sensor", "Request/FailedTransportError"} }));
        SessionCV_.Set(sensorsRegistry->IntGauge({ DatabaseLabel_,                  {"sensor", "SessionBalancer/Variation"} }));
        GRpcInFlight_.Set(sensorsRegistry->IntGauge({ DatabaseLabel_,               {"sensor", "Grpc/InFlight"} }));

        RequestLatency_.Set(sensorsRegistry->HistogramRate({ DatabaseLabel_, {"sensor", "Request/Latency"} },
            ExponentialHistogram(20, 2, 1)));
        ResultSize_.Set(sensorsRegistry->HistogramRate({ DatabaseLabel_, {"sensor", "Request/ResultSize"} },
            ExponentialHistogram(20, 2, 32)));
    }

    void IncDiscoveryDuePessimization() {
        DiscoveryDuePessimization_.Inc();
    }

    void IncDiscoveryDueExpiration() {
        DiscoveryDueExpiration_.Inc();
    }

    void IncDiscoveryFailDueTransportError() {
        DiscoveryFailDueTransportError_.Inc();
    }

    void IncReqFailQueueOverflow() {
        RequestFailDueQueueOverflow_.Inc();
    }

    void IncReqFailNoEndpoint() {
        RequestFailDueNoEndpoint_.Inc();
    }

    void IncReqFailDueTransportError() {
        RequestFailDueTransportError_.Inc();
    }

    void IncRequestLatency(TDuration duration) {
        RequestLatency_.Record(duration.MilliSeconds());
    }

    void IncResultSize(const size_t& size) {
        ResultSize_.Record(size);
    }

    void IncCounter(const std::string& sensor) {
        if (auto registry = MetricRegistryPtr_.Get()) {
            registry->Counter({ {"database", Database_}, {"sensor", sensor} })->Add(1);
        }
    }

    void SetSessionCV(std::uint32_t cv) {
        SessionCV_.SetValue(cv);
    }

    void IncGRpcInFlight() {
        GRpcInFlight_.Inc();
    }

    void DecGRpcInFlight() {
        GRpcInFlight_.Dec();
    }

    TEndpointElectorStatCollector GetEndpointElectorStatCollector() {
        if (auto registry = MetricRegistryPtr_.Get()) {
            auto endpointCoint = registry->IntGauge({ DatabaseLabel_,      {"sensor", "Endpoints/Total"} });
            auto pessimizationRatio = registry->IntGauge({ DatabaseLabel_, {"sensor", "Endpoints/BadRatio"} });
            auto activeEndpoints = registry->IntGauge({ DatabaseLabel_,    {"sensor", "Endpoints/Good"} });
            return TEndpointElectorStatCollector(endpointCoint, pessimizationRatio, activeEndpoints);
        }

        return TEndpointElectorStatCollector();
    }

    TSessionPoolStatCollector GetSessionPoolStatCollector(const std::string& clientType) {
        if (auto registry = MetricRegistryPtr_.Get()) {
            auto activeSessions = registry->IntGauge({ DatabaseLabel_, {"ydb_client", clientType},
                {"sensor", "Sessions/InUse"} });
            auto inPoolSessions = registry->IntGauge({ DatabaseLabel_, {"ydb_client", clientType},
                {"sensor", "Sessions/InPool"} });
            auto fakeSessions = registry->Rate({ DatabaseLabel_, {"ydb_client", clientType},
                {"sensor", "Sessions/SessionsLimitExceeded"} });
            auto waiters = registry->IntGauge({ DatabaseLabel_, {"ydb_client", clientType},
                {"sensor", "Sessions/WaitForReturn"} });

            return TSessionPoolStatCollector(activeSessions, inPoolSessions, fakeSessions, waiters);
        }

        return TSessionPoolStatCollector();
    }

    TClientStatCollector GetClientStatCollector(const std::string& clientType) {
        if (auto registry = MetricRegistryPtr_.Get()) {
            std::shared_ptr<NMetrics::IRate> cacheMiss = nullptr;
            std::shared_ptr<NMetrics::IRate> sessionRemovedDueBalancing = nullptr;
            std::shared_ptr<NMetrics::IRate> requestMigrated = nullptr;

            if (clientType == "Table") {
                cacheMiss = registry->Rate({ DatabaseLabel_, {"ydb_client", clientType},
                    {"sensor", "Request/ClientQueryCacheMiss"} });
                sessionRemovedDueBalancing = registry->Rate({ DatabaseLabel_, {"ydb_client", clientType},
                    {"sensor", "SessionBalancer/SessionsRemoved"} });
                requestMigrated = registry->Rate({ DatabaseLabel_, {"ydb_client", clientType},
                    {"sensor", "SessionBalancer/RequestsMigrated"} });
            }

            auto querySize = registry->HistogramRate({ DatabaseLabel_, {"ydb_client", clientType},
                {"sensor", "Request/QuerySize"} }, ExponentialHistogram(20, 2, 32));
            auto paramsSize = registry->HistogramRate({ DatabaseLabel_, {"ydb_client", clientType},
                {"sensor", "Request/ParamsSize"} }, ExponentialHistogram(10, 2, 32));

            return TClientStatCollector(cacheMiss, querySize, paramsSize, sessionRemovedDueBalancing, requestMigrated,
                TClientRetryOperationStatCollector(MetricRegistryPtr_.Get(), Database_, clientType));
        }

        return TClientStatCollector();
    }

    bool IsCollecting() {
        return MetricRegistryPtr_.Get() != nullptr;
    }

    void IncSessionsOnHost(const std::string& host);
    void DecSessionsOnHost(const std::string& host);

    void IncTransportErrorsByHost(const std::string& host);

    void IncGRpcInFlightByHost(const std::string& host);
    void DecGRpcInFlightByHost(const std::string& host);

private:
    const std::string Database_;
    const NMetrics::TLabel DatabaseLabel_;

    TAtomicPointer<NMetrics::IMetricsProvider> MetricRegistryPtr_;

    TAtomicCounter<NMetrics::IRate> DiscoveryDuePessimization_;
    TAtomicCounter<NMetrics::IRate> DiscoveryDueExpiration_;
    TAtomicCounter<NMetrics::IRate> RequestFailDueQueueOverflow_;
    TAtomicCounter<NMetrics::IRate> RequestFailDueNoEndpoint_;
    TAtomicCounter<NMetrics::IRate> RequestFailDueTransportError_;
    TAtomicCounter<NMetrics::IRate> DiscoveryFailDueTransportError_;
    TAtomicCounter<NMetrics::IIntGauge> SessionCV_;
    TAtomicCounter<NMetrics::IIntGauge> GRpcInFlight_;
    TAtomicHistogram<NMetrics::IHistogram> RequestLatency_;
    TAtomicHistogram<NMetrics::IHistogram> ResultSize_;
};

} // namespace NSdkStats
} // namespace NYdb
