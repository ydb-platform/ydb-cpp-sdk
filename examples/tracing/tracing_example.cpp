#include <ydb-cpp-sdk/client/tracing/otel_tracer.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/exporters/jaeger/jaeger_exporter.h>
#include <ydb-cpp-sdk/client/driver.h>

int main() {
    auto exporter = opentelemetry::exporter::jaeger::JaegerExporterFactory::Create();
    auto provider = opentelemetry::sdk::trace::TracerProviderFactory::Create(std::move(exporter));
    auto otelTracer = provider->GetTracer("ydb-cpp-sdk");

    auto ydbTracer = std::make_shared<NYdb::NTracing::TOpenTelemetryTracer>(otelTracer);

    auto driver = NYdb::TDriver(
        NYdb::TDriverConfig()
            .SetEndpoint("localhost:2136")
            .SetDatabase("/local")
            .SetTracer(ydbTracer)
    );

    auto client = NYdb::NTable::TTableClient(driver);
    auto session = client.CreateSession().GetValueSync();
    session.ExecuteDataQuery("SELECT 1", NYdb::NTable::TTxControl::BeginTx().CommitTx()).GetValueSync();

    return 0;
}
