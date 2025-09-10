#pragma once

#include <ydb-cpp-sdk/client/types/ydb.h>

#include <src/client/impl/internal/internal_header.h>
#include <src/client/impl/internal/common/types.h>

#include <src/client/types/core_facility/core_facility.h>

#include <library/cpp/threading/future/future.h>
#include <library/cpp/logger/log.h>


namespace NYdb::inline V3 {

class TDbDriverState;
struct TListEndpointsResult;

namespace NMetrics {
    class IMetricsProvider;
}

class IInternalClient {
public:
    virtual NThreading::TFuture<TListEndpointsResult> GetEndpoints(std::shared_ptr<TDbDriverState> dbState) = 0;
    virtual void AddPeriodicTask(TPeriodicCb&& cb, TDuration period) = 0;
#ifndef YDB_GRPC_BYPASS_CHANNEL_POOL
    virtual void DeleteChannels(const std::vector<std::string>& endpoints) = 0;
#endif
    virtual TBalancingPolicy::TImpl GetBalancingSettings() const = 0;
    virtual bool StartStatCollecting(std::shared_ptr<NMetrics::IMetricsProvider> metricsProvider) = 0;
    virtual std::shared_ptr<NMetrics::IMetricsProvider> GetMetricRegistry() = 0;
    virtual const TLog& GetLog() const = 0;
};

} // namespace NYdb
