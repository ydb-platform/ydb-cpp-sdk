#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace NYdb {
namespace NTracing {

using TAttributeMap = std::unordered_map<std::string, std::string>;

// Контекст с идентификаторами трассировки
class TTraceContext {
public:
    TTraceContext(std::string traceId, std::string spanId, std::string parentSpanId = "")
        : TraceId_(std::move(traceId))
        , SpanId_(std::move(spanId))
        , ParentSpanId_(std::move(parentSpanId))
    {}

    const std::string& GetTraceId() const { return TraceId_; }
    const std::string& GetSpanId() const { return SpanId_; }
    const std::string& GetParentSpanId() const { return ParentSpanId_; }

    // Генерация нового контекста (создает новые уникальные traceId и spanId)
    static std::shared_ptr<TTraceContext> GenerateNew();

    // Создание дочернего контекста (новый spanId, тот же traceId)
    std::shared_ptr<TTraceContext> CreateChild() const;

    // Формирование W3C traceparent ("00-traceId-spanId-01")
    std::string ToTraceParent() const;

private:
    std::string TraceId_;
    std::string SpanId_;
    std::string ParentSpanId_;
};


class ISpan {
public:
    virtual ~ISpan() = default;

    virtual void AddAttribute(const std::string& key, const std::string& value) = 0;
    virtual void AddEvent(const std::string& name, const TAttributeMap& attributes = {}) = 0;
    virtual void SetStatus(bool isError, const std::string& description = "") = 0;
    virtual void End() = 0;

    virtual const TTraceContext& GetContext() const = 0;
};


class ITracer {
public:
    virtual ~ITracer() = default;

    // Создать новый спан с именем, атрибутами и опциональным родительским контекстом
    virtual std::unique_ptr<ISpan> StartSpan(
        const std::string& name,
        const TAttributeMap& attributes = {},
        std::shared_ptr<TTraceContext> parentContext = nullptr) = 0;

    // Получить текущий контекст
    virtual std::shared_ptr<TTraceContext> GetCurrentContext() const = 0;

    // Получить W3C traceparent текущего контекста (если есть)
    virtual std::string GetCurrentTraceParent() const {
        if (auto ctx = GetCurrentContext()) {
            return ctx->ToTraceParent();
        }
        return "";
    }
};

} // namespace NTracing
} // namespace NYdb
