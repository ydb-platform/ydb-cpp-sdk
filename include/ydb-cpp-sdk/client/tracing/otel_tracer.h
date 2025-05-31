#pragma once

#include <ydb-cpp-sdk/client/tracing/tracer.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/common/attribute_value.h>

namespace NYdb::inline V3 {
namespace NTracing {

    class TOpenTelemetrySpan : public ISpan {
    public:
        explicit TOpenTelemetrySpan(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span)
            : Span_(std::move(span)) {}

        void AddAttribute(const std::string& key, const std::string& value) override {
            if (Span_) {
                Span_->SetAttribute(key, value);
            }
        }

        void End() override {
            if (Span_) {
                Span_->End();
            }
        }

    private:
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> Span_;
    };

    class TOpenTelemetryTracer : public ITracer {
    public:
        explicit TOpenTelemetryTracer(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer)
            : Tracer_(std::move(tracer)) {}

        std::unique_ptr<ISpan> StartSpan(const std::string& name) override {
            if (!Tracer_) {
                return nullptr;
            }
            auto span = Tracer_->StartSpan(name);
            return std::make_unique<TOpenTelemetrySpan>(std::move(span));
        }

    private:
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> Tracer_;
    };

} // namespace NTracing
} // namespace NYdb
