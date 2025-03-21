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

// Создаем систему мониторинга
auto monitoringSystem = TMonitoringSystemFactory::CreatePrometheus("localhost:9090");
TMetricsContext::Instance().SetMonitoringSystem(std::move(monitoringSystem));

// Создаем метрику
std::unordered_map<std::string, std::string> labels = {
    {"operation", "query"},
    {"status", "success"}
};
TPrometheusMetric metric("ydb_query_count", "1", labels);

// Записываем метрику
TMetricsContext::Instance().RecordMetric(metric);
```

### OpenTelemetry

```cpp
#include <ydb-cpp-sdk/client/monitoring/metrics.h>
#include <ydb-cpp-sdk/client/monitoring/impl/opentelemetry.h>

// Создаем систему мониторинга
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
- `ydb_query_latency` - задержка запросов
- `ydb_error_count` - количество ошибок
- `ydb_connection_count` - количество активных соединений
- `ydb_session_count` - количество активных сессий

## Примеры

Полные примеры использования различных систем мониторинга можно найти в директории `examples/monitoring/`:

- `prometheus_example.cpp` - пример использования Prometheus
- `opentelemetry_example.cpp` - пример использования OpenTelemetry
- `datadog_example.cpp` - пример использования Datadog
- `newrelic_example.cpp` - пример использования New Relic
- `appdynamics_example.cpp` - пример использования AppDynamics
- `victoriametrics_example.cpp` - пример использования Victoria Metrics
- `solomon_example.cpp` - пример использования Yandex Solomon 