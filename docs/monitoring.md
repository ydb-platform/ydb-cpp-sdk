# Мониторинг в YDB C++ SDK

YDB C++ SDK поддерживает различные системы мониторинга для сбора метрик. Вы можете выбрать одну или несколько систем мониторинга при сборке проекта.

## Поддерживаемые системы мониторинга

- Prometheus
- OpenTelemetry
- Datadog
- New Relic
- AppDynamics
- Victoria Metrics
- Yandex Solomon (по умолчанию)

## Настройка сборки

При сборке проекта вы можете включить поддержку нужных систем мониторинга с помощью CMake опций:

```cmake
cmake -DYDB_SDK_MONITORING_PROMETHEUS=ON \
      -DYDB_SDK_MONITORING_OPENTELEMETRY=ON \
      -DYDB_SDK_MONITORING_DATADOG=ON \
      -DYDB_SDK_MONITORING_NEWRELIC=ON \
      -DYDB_SDK_MONITORING_APPDYNAMICS=ON \
      -DYDB_SDK_MONITORING_VICTORIAMETRICS=ON \
      -DYDB_SDK_MONITORING_SOLOMON=ON \
      ..
```

## Использование

### Prometheus

```cpp
#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/prometheus.h>
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>

// Создаем систему мониторинга
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

    // Метрика задержки запроса
    auto latency = result.GetExecutionTime().MilliSeconds();
    TPrometheusMetric queryLatency("ydb_query_latency", std::to_string(latency), labels);
    TMetricsContext::Instance().RecordMetric(queryLatency);

    // Метрика количества строк в результате
    auto rowCount = result.GetResultSet(0).RowsCount();
    TPrometheusMetric resultRows("ydb_result_rows", std::to_string(rowCount), labels);
    TMetricsContext::Instance().RecordMetric(resultRows);

    // Принудительно отправляем метрики
    TMetricsContext::Instance().Flush();

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
}
```

### OpenTelemetry

```cpp
#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/opentelemetry.h>
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>

// Создаем систему мониторинга
auto monitoringSystem = TMonitoringSystemFactory::CreateOpenTelemetry("localhost:4317");
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
    TOpenTelemetryMetric queryCount("ydb_query_count", "1", labels);
    TMetricsContext::Instance().RecordMetric(queryCount);

    // Метрика задержки запроса
    auto latency = result.GetExecutionTime().MilliSeconds();
    TOpenTelemetryMetric queryLatency("ydb_query_latency", std::to_string(latency), labels);
    TMetricsContext::Instance().RecordMetric(queryLatency);

    // Метрика количества строк в результате
    auto rowCount = result.GetResultSet(0).RowsCount();
    TOpenTelemetryMetric resultRows("ydb_result_rows", std::to_string(rowCount), labels);
    TMetricsContext::Instance().RecordMetric(resultRows);

    // Принудительно отправляем метрики
    TMetricsContext::Instance().Flush();

} catch (const TYdbErrorException& e) {
    // В случае ошибки отправляем метрику ошибки
    std::unordered_map<std::string, std::string> errorLabels = {
        {"operation", "query"},
        {"status", "error"},
        {"database", "/local"},
        {"error_type", e.GetStatus().ToString()}
    };
    TOpenTelemetryMetric errorCount("ydb_error_count", "1", errorLabels);
    TMetricsContext::Instance().RecordMetric(errorCount);
    TMetricsContext::Instance().Flush();
}
```

### Datadog

```cpp
#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/datadog.h>

// Создаем систему мониторинга
auto monitoringSystem = TMonitoringSystemFactory::CreateDatadog("your-api-key");
TMetricsContext::Instance().SetMonitoringSystem(std::move(monitoringSystem));

// Создаем метрику
std::unordered_map<std::string, std::string> labels = {
    {"operation", "query"},
    {"status", "success"}
};
TDatadogMetric metric("ydb.query.count", "1", labels);

// Записываем метрику
TMetricsContext::Instance().RecordMetric(metric);
```

### New Relic

```cpp
#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/newrelic.h>

// Создаем систему мониторинга
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
```

### AppDynamics

```cpp
#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/appdynamics.h>

// Создаем систему мониторинга
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
```

### Victoria Metrics

```cpp
#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/victoriametrics.h>

// Создаем систему мониторинга
auto monitoringSystem = TMonitoringSystemFactory::CreateVictoriaMetrics("http://localhost:8428");
TMetricsContext::Instance().SetMonitoringSystem(std::move(monitoringSystem));

// Создаем метрику
std::unordered_map<std::string, std::string> labels = {
    {"operation", "query"},
    {"status", "success"}
};
TVictoriaMetricsMetric metric("ydb_query_count", "1", labels);

// Записываем метрику
TMetricsContext::Instance().RecordMetric(metric);
```

## Доступные метрики

YDB C++ SDK собирает следующие метрики:

- `ydb_query_count` - количество запросов
- `ydb_query_latency` - задержка запросов (в миллисекундах)
- `ydb_error_count` - количество ошибок
- `ydb_result_rows` - количество строк в результате запроса
- `ydb_connection_count` - количество активных соединений
- `ydb_session_count` - количество активных сессий

Каждая метрика может иметь следующие метки (labels):
- `operation` - тип операции (query, transaction и т.д.)
- `status` - статус операции (success, error)
- `database` - имя базы данных
- `error_type` - тип ошибки (только для метрик ошибок)

## Примеры

Полные примеры использования различных систем мониторинга можно найти в директории `examples/monitoring/`:

- `prometheus_example.cpp` - пример использования Prometheus с реальными запросами к YDB
- `opentelemetry_example.cpp` - пример использования OpenTelemetry с реальными запросами к YDB
- `datadog_example.cpp` - пример использования Datadog с реальными запросами к YDB
- `newrelic_example.cpp` - пример использования New Relic с реальными запросами к YDB
- `appdynamics_example.cpp` - пример использования AppDynamics с реальными запросами к YDB
- `victoriametrics_example.cpp` - пример использования Victoria Metrics с реальными запросами к YDB
- `solomon_example.cpp` - пример использования Yandex Solomon с реальными запросами к YDB 