#pragma once

#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <appdynamics/appdynamics.h>

namespace NYdb::NMonitoring {

class TAppDynamicsMetric : public IMetric {
public:
    TAppDynamicsMetric(const std::string& name, const std::string& value, 
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

class TAppDynamicsMonitoringSystem : public IMonitoringSystem {
public:
    explicit TAppDynamicsMonitoringSystem(const std::string& controllerUrl)
        : ControllerUrl_(controllerUrl)
    {
        Initialize();
    }

    void RecordMetric(const IMetric& metric) override;
    void Flush() override;
    std::string GetSystemName() const override { return "appdynamics"; }

private:
    void Initialize();
    std::string ControllerUrl_;
    AppDynamics* Agent_;
};

} // namespace NYdb::NMonitoring 