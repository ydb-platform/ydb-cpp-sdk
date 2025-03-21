#pragma once

#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

namespace NYdb::NMonitoring {

class TPrometheusMetric : public IMetric {
public:
    TPrometheusMetric(const std::string& name, const std::string& value, 
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

class TPrometheusMonitoringSystem : public IMonitoringSystem {
public:
    explicit TPrometheusMonitoringSystem(const std::string& endpoint)
        : Exposer_(endpoint)
        , Registry_(std::make_shared<prometheus::Registry>())
    {
        Exposer_.RegisterCollectable(Registry_);
    }

    void RecordMetric(const IMetric& metric) override;
    void Flush() override;

private:
    prometheus::Exposer Exposer_;
    std::shared_ptr<prometheus::Registry> Registry_;
};

} // namespace NYdb::NMonitoring 