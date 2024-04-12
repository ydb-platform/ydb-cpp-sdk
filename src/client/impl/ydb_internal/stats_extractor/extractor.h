#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

#include <src/client/impl/ydb_internal/internal_client/client.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/client/ydb_extension/extension.h>

#include <ydb-cpp-sdk/library/monlib/metrics/metric_registry.h>
=======
#include <src/client/ydb_extension/extension.h>

#include <src/library/monlib/metrics/metric_registry.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYdb {

class TStatsExtractor: public NSdkStats::IStatApi {
public:

    TStatsExtractor(std::shared_ptr<IInternalClient> client)
    : Client_(client)
    { }

    virtual void SetMetricRegistry(::NMonitoring::IMetricRegistry* sensorsRegistry) override {
        auto strong = Client_.lock();
        if (strong) {
            strong->StartStatCollecting(sensorsRegistry);
        } else {
            return;
        }
    }

    void Accept(NMonitoring::IMetricConsumer* consumer) const override {

        auto strong = Client_.lock();
        if (strong) {
            auto sensorsRegistry = strong->GetMetricRegistry();
            Y_ABORT_UNLESS(sensorsRegistry, "TMetricRegistry is null in Stats Extractor");
            sensorsRegistry->Accept(TInstant::Zero(), consumer);
        } else {
             throw NSdkStats::DestroyedClientException();
        }
    }
private:
    std::weak_ptr<IInternalClient> Client_;
};

} // namespace NYdb
