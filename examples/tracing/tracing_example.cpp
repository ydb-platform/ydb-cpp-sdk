#include <ydb-cpp-sdk/client/tracing/otel_tracer.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/exporters/jaeger/jaeger_exporter.h>
#include <ydb-cpp-sdk/client/driver.h>

int main() {
    // 1. Настройка OpenTelemetry с экспортером в Jaeger
    auto exporter = opentelemetry::exporter::jaeger::JaegerExporterFactory::Create();
    auto provider = opentelemetry::sdk::trace::TracerProviderFactory::Create(std::move(exporter));
    auto otel_tracer = provider->GetTracer("ydb-cpp-sdk");

    // 2. Создание адаптера для YDB SDK
    auto ydb_tracer = std::make_shared<NYdb::NTracing::TOpenTelemetryTracer>(otel_tracer);

    // 3. Инициализация драйвера YDB с трейсером
    auto driver = NYdb::TDriver(
        NYdb::TDriverConfig()
            .SetEndpoint("grpc://localhost:2136")
            .SetDatabase("/local")
            .SetTracer(ydb_tracer)
    );

    // 4. Тестовый запрос (спан создастся автоматически внутри SDK)
    auto client = NYdb::NTable::TTableClient(driver);
    auto session = client.CreateSession().GetValueSync();
    session.ExecuteDataQuery("SELECT 1", NYdb::NTable::TTxControl::BeginTx().CommitTx()).GetValueSync();

    return 0;
}
