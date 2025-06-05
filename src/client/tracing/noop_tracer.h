#pragma once

#include <ydb-cpp-sdk/client/tracing/tracer.h>

namespace NYdb::inline V3 {
namespace NTracing {

    class TNoopSpan : public ISpan {
    public:
        void AddAttribute(const std::string&, const std::string&) override {}
        void AddEvent(const std::string&, const TAttributeMap& = {}) override {}
        void SetStatus(bool, const std::string& = "") override {}
        void End() override {}

        const TTraceContext& GetContext() const override {
            static TTraceContext emptyContext("", "");
            return emptyContext;
        }
    };

    class TNoopTracer : public ITracer {
    public:
        std::unique_ptr<ISpan> StartSpan(
            const std::string&,
            const TAttributeMap& = {},
            std::shared_ptr<TTraceContext> = nullptr) override
        {
            return std::make_unique<TNoopSpan>();
        }

        std::shared_ptr<TTraceContext> GetCurrentContext() const override {
            return nullptr;
        }
    };

} // namespace NTracing
} // namespace NYdb
