#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/opentelemetry.h>
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <iostream>

using namespace NYdb;
using namespace NYdb::NMonitoring;

int main() {
    // Создаем систему мониторинга OpenTelemetry
    auto monitoringSystem = TMonitoringSystemFactory::CreateOpenTelemetry("localhost:4317");
    TMetricsContext::Instance().SetMonitoringSystem(std::move(monitoringSystem));

    // Создаем метрику
    std::unordered_map<std::string, std::string> labels = {
        {"operation", "query"},
        {"status", "success"}
    };
    TOpenTelemetryMetric metric("ydb_query_count", "1", labels);

    // Записываем метрику
    TMetricsContext::Instance().RecordMetric(metric);

    // Принудительно отправляем метрики
    TMetricsContext::Instance().Flush();

    return 0;
} 