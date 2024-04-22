#include <ydb-cpp-sdk/library/monlib/metrics/metric_consumer.h>

#include <ydb-cpp-sdk/util/system/yassert.h>

namespace NMonitoring {
    void IMetricConsumer::OnLabel(ui32 name, ui32 value) {
        Y_UNUSED(name, value);
        Y_ENSURE(false, "Not implemented");
    }

    std::pair<ui32, ui32> IMetricConsumer::PrepareLabel(std::string_view name, std::string_view value) {
        Y_UNUSED(name, value);
        Y_ENSURE(false, "Not implemented");
    }
}
