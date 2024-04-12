#pragma once

#include "pre_mon_page.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/monlib/metrics/metric_registry.h>
=======
#include <src/library/monlib/metrics/metric_registry.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NMonitoring {
    // For now this class can only enumerate all metrics without any grouping or serve JSON/Spack/Prometheus
    class TMetricRegistryPage: public TPreMonPage {
    public:
        TMetricRegistryPage(const std::string& path, const std::string& title, TAtomicSharedPtr<IMetricSupplier> registry)
            : TPreMonPage(path, title)
            , Registry_(registry)
            , RegistryRawPtr_(Registry_.Get())
        {
        }

        TMetricRegistryPage(const std::string& path, const std::string& title, IMetricSupplier* registry)
            : TPreMonPage(path, title)
            , RegistryRawPtr_(registry)
        {
        }

        void Output(NMonitoring::IMonHttpRequest& request) override;
        void OutputText(IOutputStream& out, NMonitoring::IMonHttpRequest&) override;

    private:
        TAtomicSharedPtr<IMetricSupplier> Registry_;
        IMetricSupplier* RegistryRawPtr_;
    };

}
