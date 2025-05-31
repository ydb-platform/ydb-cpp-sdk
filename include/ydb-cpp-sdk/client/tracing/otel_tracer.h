#pragma once

#include "tracer.h"

#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>
#include <opentelemetry/context/runtime_context.h>
#include <opentelemetry/common/key_value_iterable_view.h>

namespace NYdb {
namespace NTracing {

class OpenTelemetrySpan : public ISpan {
public:
    OpenTelemetrySpan(opentelemetry::trace::Span span, std::shared_ptr<TTraceContext> context)
        : span_(std::move(span)), context_(std::move(context)) {}

    void AddAttribute(const std::string& key, const std::string& value) override {
        span_.SetAttribute(key, value);
    }

    void AddEvent(const std::string& name, const TAttributeMap& attributes = {}) override {
        // Преобразуем std::unordered_map в вектор KeyValue для OpenTelemetry
        std::vector<opentelemetry::common::KeyValue> otelAttributes;
        otelAttributes.reserve(attributes.size());
        for (const auto& [k, v] : attributes) {
            otelAttributes.emplace_back(k, v);
        }
        span_.AddEvent(name, otelAttributes);
    }

    void SetStatus(bool isError, const std::string& description = "") override {
        span_.SetStatus(isError ? opentelemetry::trace::StatusCode::kError
                                : opentelemetry::trace::StatusCode::kOk,
                        description);
    }

    void End() override {
        span_.End();
    }

    const TTraceContext& GetContext() const override {
        return *context_;
    }

private:
    opentelemetry::trace::Span span_;
    std::shared_ptr<TTraceContext> context_;
};


class OpenTelemetryTracer : public ITracer {
public:
    explicit OpenTelemetryTracer(std::shared_ptr<opentelemetry::trace::Tracer> tracer)
        : tracer_(std::move(tracer)) {}

    std::unique_ptr<ISpan> StartSpan(
        const std::string& name,
        const TAttributeMap& attributes = {},
        std::shared_ptr<TTraceContext> parentContext = nullptr
    ) override {
        opentelemetry::context::Context otelContext;

        if (parentContext) {
            auto traceId = opentelemetry::trace::TraceId::FromHex(parentContext->GetTraceId());
            auto spanId = opentelemetry::trace::SpanId::FromHex(parentContext->GetSpanId());

            auto spanContext = opentelemetry::trace::SpanContext::Create(
                traceId,
                spanId,
                opentelemetry::trace::TraceFlags::kIsSampled,
                false // remote
            );

            otelContext = opentelemetry::context::Context{}.SetValue(
                opentelemetry::trace::kSpanKey,
                opentelemetry::trace::Span{spanContext});
        }

        auto span = tracer_->StartSpan(name, attributes, otelContext);

        auto context = parentContext ? parentContext->CreateChild() : TTraceContext::GenerateNew();

        return std::make_unique<OpenTelemetrySpan>(std::move(span), std::move(context));
    }

    std::shared_ptr<TTraceContext> GetCurrentContext() const override {
        auto currentSpan = opentelemetry::trace::GetCurrentSpan();
        if (!currentSpan.IsValid()) {
            return nullptr;
        }
        auto spanContext = currentSpan.GetContext();

        return std::make_shared<TTraceContext>(
            spanContext.trace_id().ToHex(),
            spanContext.span_id().ToHex(),
            spanContext.IsValid() ? spanContext.trace_id().ToHex() : "");
    }

private:
    std::shared_ptr<opentelemetry::trace::Tracer> tracer_;
};

} // namespace NTracing
} // namespace NYdb
