#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/appdynamics.h>
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <iostream>

using namespace NYdb;
using namespace NYdb::NMonitoring;

int main() {
    // Создаем систему мониторинга AppDynamics
    auto monitoringSystem = TMonitoringSystemFactory::CreateAppDynamics("http://your-controller:8090");
    TMetricsContext::Instance().SetMonitoringSystem(std::move(monitoringSystem));

    // Создаем метрику
    std::unordered_map<std::string, std::string> labels = {
        {"operation", "query"},
        {"status", "success"}
    };
    TAppDynamicsMetric metric("Custom Metrics|YDB|Query Count", "1", labels);

    // Записываем метрику
    TMetricsContext::Instance().RecordMetric(metric);

    // Принудительно отправляем метрики
    TMetricsContext::Instance().Flush();

    return 0;
} 