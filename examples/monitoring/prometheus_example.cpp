#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/prometheus.h>
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>
#include <ydb-cpp-sdk/client/table/result.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace NYdb;
using namespace NYdb::NMonitoring;
using namespace NYdb::NTable;

int main() {
    // Создаем систему мониторинга Prometheus
    auto monitoringSystem = TMonitoringSystemFactory::CreatePrometheus("localhost:9090");
    TMetricsContext::Instance().SetMonitoringSystem(std::move(monitoringSystem));

    // Создаем драйвер YDB
    TDriverConfig config;
    config.SetEndpoint("localhost:2135");
    config.SetDatabase("/local");
    TDriver driver(config);

    // Создаем клиент таблиц
    TTableClient client(driver);

    try {
        // Выполняем запрос
        auto result = client.ExecuteDataQuery(R"(
            SELECT 1 + 1 AS result;
        )", TTxControl::BeginTx(TTxSettings::SerializableRW()).CommitTx());

        // Создаем метрики на основе результата запроса
        std::unordered_map<std::string, std::string> labels = {
            {"operation", "query"},
            {"status", "success"},
            {"database", "/local"}
        };

        // Метрика количества запросов
        TPrometheusMetric queryCount("ydb_query_count", "1", labels);
        TMetricsContext::Instance().RecordMetric(queryCount);

        // Метрика задержки запроса (в миллисекундах)
        auto latency = result.GetExecutionTime().MilliSeconds();
        TPrometheusMetric queryLatency("ydb_query_latency", std::to_string(latency), labels);
        TMetricsContext::Instance().RecordMetric(queryLatency);

        // Метрика количества строк в результате
        auto rowCount = result.GetResultSet(0).RowsCount();
        TPrometheusMetric resultRows("ydb_result_rows", std::to_string(rowCount), labels);
        TMetricsContext::Instance().RecordMetric(resultRows);

        // Принудительно отправляем метрики
        TMetricsContext::Instance().Flush();

        std::cout << "Query executed successfully. Metrics sent to Prometheus." << std::endl;

    } catch (const TYdbErrorException& e) {
        // В случае ошибки отправляем метрику ошибки
        std::unordered_map<std::string, std::string> errorLabels = {
            {"operation", "query"},
            {"status", "error"},
            {"database", "/local"},
            {"error_type", e.GetStatus().ToString()}
        };
        TPrometheusMetric errorCount("ydb_error_count", "1", errorLabels);
        TMetricsContext::Instance().RecordMetric(errorCount);
        TMetricsContext::Instance().Flush();

        std::cerr << "Error executing query: " << e.GetStatus() << std::endl;
        return 1;
    }

    // Ждем некоторое время, чтобы метрики успели отправиться
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
} 