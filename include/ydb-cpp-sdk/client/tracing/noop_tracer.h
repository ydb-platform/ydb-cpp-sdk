#pragma once

#include <ydb-cpp-sdk/client/tracing/tracer.h>

namespace NYdb::inline V3 {
namespace NTracing {

class TNoopSpan : public ISpan {
public:
    void AddAttribute(const std::string&, const std::string&) override {}
    void End() override {}
};

class TNoopTracer : public ITracer {
public:
    std::unique_ptr<ISpan> StartSpan(const std::string&) override {
        return std::make_unique<TNoopSpan>();
    }
};

} // namespace NTracing
} // namespace NYdb
