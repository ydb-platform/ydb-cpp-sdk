#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/newrelic.h>
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <iostream>

using namespace NYdb;
using namespace NYdb::NMonitoring;

int main() {
    // Создаем систему мониторинга New Relic
    auto monitoringSystem = TMonitoringSystemFactory::CreateNewRelic("your-license-key");
    TMetricsContext::Instance().SetMonitoringSystem(std::move(monitoringSystem));

    // Создаем метрику
    std::unordered_map<std::string, std::string> labels = {
        {"operation", "query"},
        {"status", "success"}
    };
    TNewRelicMetric metric("Custom/ydb/query_count", "1", labels);

    // Записываем метрику
    TMetricsContext::Instance().RecordMetric(metric);

    // Принудительно отправляем метрики
    TMetricsContext::Instance().Flush();

    return 0;
} 