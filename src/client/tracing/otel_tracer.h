#pragma once

#include "tracer.h"

#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>
#include <opentelemetry/context/runtime_context.h>
#include <opentelemetry/common/key_value_iterable_view.h>

namespace NYdb {
namespace NTracing {

class TOpenTelemetrySpan : public ISpan {
public:
    OpenTelemetrySpan(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span,
        std::shared_ptr<TTraceContext> context
    ) : span_(std::move(span)), context_(std::move(context)) {}

    void AddAttribute(const std::string& key, const std::string& value) override {
        span_->SetAttribute(key, value);
    }

    void AddEvent(const std::string& name, const TAttributeMap& attributes = {}) override {
        // Преобразование атрибутов в формат OpenTelemetry
        std::vector<std::pair<std::string, std::string>> kvPairs;
        kvPairs.reserve(attributes.size());
        for (const auto& [k, v] : attributes) {
            kvPairs.emplace_back(k, v);
        }

        // Создание ивента
        span_->AddEvent(name, opentelemetry::common::MakeKeyValueIterableView(kvPairs));
    }

    void SetStatus(bool isError, const std::string& description = "") override {
        span_->SetStatus(isError ? opentelemetry::trace::StatusCode::kError
                                 : opentelemetry::trace::StatusCode::kOk,
                         description);
    }

    void End() override {
        span_->End();
    }

    const TTraceContext& GetContext() const override {
        return *context_;
    }

private:
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span_;
    std::shared_ptr<TTraceContext> context_;
};

class TOpenTelemetryTracer : public ITracer {
public:
    explicit OpenTelemetryTracer(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer
    ) : tracer_(std::move(tracer)) {}

    std::unique_ptr<ISpan> StartSpan(
        const std::string& name,
        const TAttributeMap& attributes = {},
        std::shared_ptr<TTraceContext> parentContext = nullptr
    ) override {
        opentelemetry::trace::StartSpanOptions options;

        // Установка родительского контекста
        if (parentContext) {
            auto traceId = opentelemetry::trace::TraceId::FromHex(parentContext->GetTraceId());
            auto spanId = opentelemetry::trace::SpanId::FromHex(parentContext->GetSpanId());

            auto parentContext = opentelemetry::trace::SpanContext(
                traceId,
                spanId,
                opentelemetry::trace::TraceFlags::kSampled,
                true
            );
            options.parent = parentContext;
        }

        // Создание спана
        auto span = tracer_->StartSpan(name, options);

        // Установка атрибутов
        for (const auto& [key, value] : attributes) {
            span->SetAttribute(key, value);
        }

        // Создание контекста
        std::shared_ptr<TTraceContext> context;
        if (parentContext) {
            context = parentContext->CreateChild();
        } else {
            context = TTraceContext::GenerateNew();
        }

        return std::make_unique<OpenTelemetrySpan>(span, context);
    }

    std::shared_ptr<TTraceContext> GetCurrentContext() const override {
        auto currentSpan = opentelemetry::trace::GetSpan(
            opentelemetry::context::RuntimeContext::GetCurrent()
        );

        if (!currentSpan->IsValid()) {
            return nullptr;
        }

        auto context = currentSpan->GetContext();
        return std::make_shared<TTraceContext>(
            context.trace_id().ToHex(),
            context.span_id().ToHex()
        );
    }

private:
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer_;
};

} // namespace NTracing
} // namespace NYdb
