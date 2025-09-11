#include <ydb-cpp-sdk/client/extension_common/extension.h>

#define INCLUDE_YDB_INTERNAL_H
#include <src/client/impl/internal/grpc_connections/grpc_connections.h>
#undef INCLUDE_YDB_INTERNAL_H

#include <src/client/metrics_providers/monlib/provider.h>


namespace NYdb::inline V3 {

void IExtension::SelfRegister(TDriver driver) {
    CreateInternalInterface(driver)->RegisterExtension(this);
}

void IExtensionApi::SelfRegister(TDriver driver) {
    CreateInternalInterface(driver)->RegisterExtensionApi(this);
}

class TStatsExtractor: public NSdkStats::IStatApi {
public:
    TStatsExtractor(std::shared_ptr<IInternalClient> client)
        : Client_(client)
    {}

    virtual void SetMetricRegistry(::NMonitoring::IMetricRegistry* sensorsRegistry) override {
        auto strong = Client_.lock();
        if (strong) {
            strong->StartStatCollecting(NMetrics::CreateMonlibMetricsProvider(sensorsRegistry));
        }
    }

    void Accept(NMonitoring::IMetricConsumer* consumer) const override {
        auto strong = Client_.lock();
        if (strong) {
            auto sensorsRegistry = strong->GetMetricRegistry();
            auto monlibRegistry = NMetrics::TryGetUnderlyingMetricsRegistry(sensorsRegistry);
            Y_ABORT_UNLESS(monlibRegistry, "IMetricRegistry is not a TMonlibMetricsProvider in Stats Extractor");
            monlibRegistry->Accept(TInstant::Zero(), consumer);
        } else {
            throw NSdkStats::DestroyedClientException();
        }
    }
private:
    std::weak_ptr<IInternalClient> Client_;
};

namespace NSdkStats {

IStatApi* IStatApi::Create(TDriver driver) {
    return new TStatsExtractor(CreateInternalInterface(driver));
}

} // namespace YSdkStats

class TDiscoveryMutator : public IDiscoveryMutatorApi {
public:
    TDiscoveryMutator(std::shared_ptr<TGRpcConnectionsImpl> driverImpl)
        : DriverImpl(driverImpl.get())
    { }

    void SetMutatorCb(TMutatorCb&& cb) override {
        DriverImpl->SetDiscoveryMutator(std::move(cb));
    }
private:
    TGRpcConnectionsImpl* DriverImpl;
};

IDiscoveryMutatorApi* IDiscoveryMutatorApi::Create(TDriver driver) {
    return new TDiscoveryMutator(CreateInternalInterface(driver));
}

} // namespace NYdb

