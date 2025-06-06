#include <ydb-cpp-sdk/client/tracing/tracer.h>
#include <random>
#include <sstream>
#include <iomanip>

namespace NYdb {
namespace NTracing {

namespace {
    std::string GenerateRandomHex(size_t length) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);

        std::ostringstream oss;
        for (size_t i = 0; i < length; ++i) {
            oss << std::hex << dis(gen);
        }
        return oss.str();
    }
}

std::shared_ptr<TTraceContext> TTraceContext::GenerateNew() {
    return std::make_shared<TTraceContext>(
            GenerateRandomHex(32),
            GenerateRandomHex(16)
    );
}

std::shared_ptr<TTraceContext> TTraceContext::CreateChild() const {
    return std::make_shared<TTraceContext>(
            TraceId_,
            GenerateRandomHex(16),
            SpanId_
    );
}

std::string TTraceContext::ToTraceParent() const {
    // Format: 00-<trace_id>-<span_id>-01
    return "00-" + TraceId_ + "-" + SpanId_ + "-01";
}

} // namespace NTracing
} // namespace NYdb
