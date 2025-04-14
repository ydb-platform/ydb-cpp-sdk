#pragma once

#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/metrics/metric_reader.h>
#include <opentelemetry/sdk/metrics/push_metric_exporter.h>

namespace NYdb::NMonitoring {

class TOpenTelemetryMetric : public IMetric {
public:
    TOpenTelemetryMetric(const std::string& name, const std::string& value, 
                        const std::unordered_map<std::string, std::string>& labels)
        : Name_(name)
        , Value_(value)
        , Labels_(labels)
    {}

    std::string GetName() const override { return Name_; }
    std::string GetValue() const override { return Value_; }
    std::unordered_map<std::string, std::string> GetLabels() const override { return Labels_; }

private:
    std::string Name_;
    std::string Value_;
    std::unordered_map<std::string, std::string> Labels_;
};

class TOpenTelemetryMonitoringSystem : public IMonitoringSystem {
public:
    explicit TOpenTelemetryMonitoringSystem(const std::string& endpoint)
        : Endpoint_(endpoint)
    {
        Initialize();
    }

    void RecordMetric(const IMetric& metric) override;
    void Flush() override;

private:
    void Initialize();
    std::string Endpoint_;
    std::shared_ptr<opentelemetry::sdk::metrics::MeterProvider> MeterProvider_;
    std::shared_ptr<opentelemetry::sdk::metrics::MetricReader> MetricReader_;
};

} // namespace NYdb::NMonitoring 