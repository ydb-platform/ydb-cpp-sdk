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

    void RecordMetric(const IMetric& metric) override {
        auto& counter = prometheus::BuildCounter()
            .Name(metric.GetName())
            .Help("YDB SDK metric")
            .Register(*Registry_)
            .Add(metric.GetLabels());

        counter.Increment(std::stod(metric.GetValue()));
    }

    void Flush() override {
        // Prometheus автоматически собирает метрики
    }

private:
    prometheus::Exposer Exposer_;
    std::shared_ptr<prometheus::Registry> Registry_;
};

std::unique_ptr<IMonitoringSystem> TMonitoringSystemFactory::CreatePrometheus(const std::string& endpoint) {
    return std::make_unique<TPrometheusMonitoringSystem>(endpoint);
}

TMetricsContext& TMetricsContext::Instance() {
    static TMetricsContext instance;
    return instance;
}

void TMetricsContext::SetMonitoringSystem(std::unique_ptr<IMonitoringSystem> system) {
    MonitoringSystem_ = std::move(system);
}

void TMetricsContext::RecordMetric(const IMetric& metric) {
    if (MonitoringSystem_) {
        MonitoringSystem_->RecordMetric(metric);
    }
}

void TMetricsContext::Flush() {
    if (MonitoringSystem_) {
        MonitoringSystem_->Flush();
    }
}

} // namespace NYdb::NMonitoring 