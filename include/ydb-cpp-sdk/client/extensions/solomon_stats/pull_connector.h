#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/client/extensions/solomon_stats/pull_connector.h
#include <ydb-cpp-sdk/client/ydb_extension/extension.h>

#include <ydb-cpp-sdk/library/monlib/metrics/metric_registry.h>
========
#include <src/client/ydb_extension/extension.h>

#include <src/library/monlib/metrics/metric_registry.h>

#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/extensions/solomon_stats/pull_connector.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/extensions/solomon_stats/pull_connector.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NSolomonStatExtension {

template <typename TMetricRegistryPtr>
class TMetricRegistryConnector: public NYdb::IExtension {
    static NMonitoring::IMetricRegistry* ToRawPtr(NMonitoring::IMetricRegistry* p) {
        return p;
    }

    static NMonitoring::IMetricRegistry* ToRawPtr(std::shared_ptr<NMonitoring::IMetricRegistry> p) {
        return p.get();
    }

    static NMonitoring::IMetricRegistry* ToRawPtr(TAtomicSharedPtr<NMonitoring::IMetricRegistry> p) {
        return p.Get();
    }

public:
    using IApi = NYdb::NSdkStats::IStatApi;

    class TParams : public TNonCopyable {
        friend class TMetricRegistryConnector;

    public:
        TParams(TMetricRegistryPtr registry)
            : Registry(registry)
        {}

    private:
        TMetricRegistryPtr Registry;
    };

    TMetricRegistryConnector(const TParams& params, IApi* api)
        : MetricRegistry_(params.Registry)
    {
        api->SetMetricRegistry(ToRawPtr(MetricRegistry_));
    }
private:
    TMetricRegistryPtr MetricRegistry_;
};

inline void AddMetricRegistry(NYdb::TDriver& driver, NMonitoring::IMetricRegistry* ptr) {
    if (!ptr)
        return;
    using TConnector = TMetricRegistryConnector<NMonitoring::IMetricRegistry*>;

    driver.AddExtension<TConnector>(TConnector::TParams(ptr));
}

inline void AddMetricRegistry(NYdb::TDriver& driver, std::shared_ptr<NMonitoring::IMetricRegistry> ptr) {
    if (!ptr)
        return;
    using TConnector = TMetricRegistryConnector<std::shared_ptr<NMonitoring::IMetricRegistry>>;

    driver.AddExtension<TConnector>(TConnector::TParams(ptr));
}

inline void AddMetricRegistry(NYdb::TDriver& driver, TAtomicSharedPtr<NMonitoring::IMetricRegistry> ptr) {
    if (!ptr)
        return;
    using TConnector = TMetricRegistryConnector<TAtomicSharedPtr<NMonitoring::IMetricRegistry>>;

    driver.AddExtension<TConnector>(TConnector::TParams(ptr));
}

} // namespace NSolomonStatExtension
