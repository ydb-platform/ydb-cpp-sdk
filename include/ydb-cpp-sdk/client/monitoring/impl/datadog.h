#pragma once

#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <datadog/tracing.h>
#include <datadog/metrics.h>

namespace NYdb::NMonitoring {

class TDatadogMetric : public IMetric {
public:
    TDatadogMetric(const std::string& name, const std::string& value, 
                   const std::unordered_map<std::string, std::string>& labels)
        : Name_(name)
        , Value_(value)
        , Labels_(labels)
    {}

    std::string GetName() const override { return Name_; }
    std::string GetValue() const override { return Value_; }
    std::unordered_map<std::string, std::string> GetLabels() const override { return Labels_; }
    std::string GetType() const override { return "counter"; }

private:
    std::string Name_;
    std::string Value_;
    std::unordered_map<std::string, std::string> Labels_;
};

class TDatadogMonitoringSystem : public IMonitoringSystem {
public:
    explicit TDatadogMonitoringSystem(const std::string& apiKey)
        : ApiKey_(apiKey)
    {
        Initialize();
    }

    void RecordMetric(const IMetric& metric) override;
    void Flush() override;
    std::string GetSystemName() const override { return "datadog"; }

private:
    void Initialize();
    std::string ApiKey_;
    std::shared_ptr<datadog::tracing::Tracer> Tracer_;
    std::shared_ptr<datadog::metrics::MetricsClient> MetricsClient_;
};

} // namespace NYdb::NMonitoring 