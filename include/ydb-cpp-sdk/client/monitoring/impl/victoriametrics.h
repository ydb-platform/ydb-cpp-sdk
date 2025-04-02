#pragma once

#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <victoriametrics/client.h>

namespace NYdb::NMonitoring {

class TVictoriaMetricsMetric : public IMetric {
public:
    TVictoriaMetricsMetric(const std::string& name, const std::string& value, 
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

class TVictoriaMetricsMonitoringSystem : public IMonitoringSystem {
public:
    explicit TVictoriaMetricsMonitoringSystem(const std::string& endpoint)
        : Endpoint_(endpoint)
    {
        Initialize();
    }

    void RecordMetric(const IMetric& metric) override;
    void Flush() override;
    std::string GetSystemName() const override { return "victoriametrics"; }

private:
    void Initialize();
    std::string Endpoint_;
    std::shared_ptr<victoriametrics::Client> Client_;
};

} // namespace NYdb::NMonitoring 