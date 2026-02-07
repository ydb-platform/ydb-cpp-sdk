#pragma once

#include <ydb-cpp-sdk/client/metrics/metrics.h>

#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/trace/tracer_provider.h>

namespace NYdb::inline V3::NMetrics {

class TOtelMetricRegistry : public IMetricRegistry {
public:
    TOtelMetricRegistry(opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> meterProvider);

    std::shared_ptr<ICounter> Counter(const std::string& name, const TLabels& labels = {}) override;
    std::shared_ptr<IGauge> Gauge(const std::string& name, const TLabels& labels = {}) override;
    std::shared_ptr<IHistogram> Histogram(const std::string& name, const std::vector<double>& buckets, const TLabels& labels = {}) override;

private:
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> MeterProvider_;
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> Meter_;
};

class TOtelTraceProvider : public ITraceProvider {
public:
    TOtelTraceProvider(opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider);

    std::shared_ptr<ITracer> GetTracer(const std::string& name) override;

private:
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> TracerProvider_;
};

} // namespace NYdb::NMetrics
