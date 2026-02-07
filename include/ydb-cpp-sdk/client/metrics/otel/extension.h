#pragma once

#include <ydb-cpp-sdk/client/metrics/otel/otel.h>
#include <ydb-cpp-sdk/client/extension_common/extension.h>

namespace NYdb::inline V3::NMetrics {

class TOtelExtension : public IExtension {
public:
    using IApi = IMetricsApi;

    struct TParams {
        opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> MeterProvider;
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> TracerProvider;
    };

    TOtelExtension(const TParams& params, IApi* api) {
        if (params.MeterProvider) {
            api->SetMetricRegistry(std::make_shared<TOtelMetricRegistry>(params.MeterProvider));
        }
        if (params.TracerProvider) {
            api->SetTraceProvider(std::make_shared<TOtelTraceProvider>(params.TracerProvider));
        }
    }
};

inline void AddOpenTelemetry(TDriver& driver
    , opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> meterProvider
    , opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider
) {
    driver.AddExtension<TOtelExtension>({meterProvider, tracerProvider});
}

} // namespace NYdb::NMetrics
