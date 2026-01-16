#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace NYdb::NMonitoring {

// Базовый класс для метрики
class IMetric {
public:
    virtual ~IMetric() = default;
    virtual std::string GetName() const = 0;
    virtual std::string GetValue() const = 0;
    virtual std::unordered_map<std::string, std::string> GetLabels() const = 0;
    virtual std::string GetType() const = 0; // counter, gauge, histogram
};

// Интерфейс для системы мониторинга
class IMonitoringSystem {
public:
    virtual ~IMonitoringSystem() = default;
    virtual void RecordMetric(const IMetric& metric) = 0;
    virtual void Flush() = 0;
    virtual std::string GetSystemName() const = 0;
};

// Фабрика для создания систем мониторинга
class TMonitoringSystemFactory {
public:
    static std::unique_ptr<IMonitoringSystem> CreatePrometheus(const std::string& endpoint);
    static std::unique_ptr<IMonitoringSystem> CreateOpenTelemetry(const std::string& endpoint);
    static std::unique_ptr<IMonitoringSystem> CreateDatadog(const std::string& apiKey);
    static std::unique_ptr<IMonitoringSystem> CreateNewRelic(const std::string& licenseKey);
    static std::unique_ptr<IMonitoringSystem> CreateAppDynamics(const std::string& controllerUrl);
    static std::unique_ptr<IMonitoringSystem> CreateVictoriaMetrics(const std::string& endpoint);
    static std::unique_ptr<IMonitoringSystem> CreateSolomon(const std::string& endpoint);
};

// Контекст для хранения метрик
class TMetricsContext {
public:
    static TMetricsContext& Instance();
    
    void SetMonitoringSystem(std::unique_ptr<IMonitoringSystem> system);
    void RecordMetric(const IMetric& metric);
    void Flush();

private:
    TMetricsContext() = default;
    std::unique_ptr<IMonitoringSystem> MonitoringSystem_;
};

// Вспомогательные функции для создания метрик
inline std::unique_ptr<IMetric> CreateCounterMetric(
    const std::string& name,
    const std::string& value,
    const std::unordered_map<std::string, std::string>& labels = {})
{
    return std::make_unique<TPrometheusMetric>(name, value, labels);
}

inline std::unique_ptr<IMetric> CreateGaugeMetric(
    const std::string& name,
    const std::string& value,
    const std::unordered_map<std::string, std::string>& labels = {})
{
    return std::make_unique<TPrometheusMetric>(name, value, labels);
}

inline std::unique_ptr<IMetric> CreateHistogramMetric(
    const std::string& name,
    const std::string& value,
    const std::unordered_map<std::string, std::string>& labels = {})
{
    return std::make_unique<TPrometheusMetric>(name, value, labels);
}

} // namespace NYdb::NMonitoring 