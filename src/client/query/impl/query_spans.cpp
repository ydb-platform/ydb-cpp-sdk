#include "query_spans.h"

namespace NYdb::inline V3::NQuery {

namespace {

void ParseEndpoint(const std::string& endpoint, std::string& host, int& port) {
    auto pos = endpoint.find(':');
    if (pos != std::string::npos) {
        host = endpoint.substr(0, pos);
        try {
            port = std::stoi(endpoint.substr(pos + 1));
        } catch (...) {
            port = 2135;
        }
    } else {
        host = endpoint;
        port = 2135;
    }
}

std::string StatusToString(EStatus status) {
    switch (status) {
        case EStatus::STATUS_UNDEFINED: return "STATUS_UNDEFINED";
        case EStatus::SUCCESS: return "SUCCESS";
        case EStatus::BAD_REQUEST: return "BAD_REQUEST";
        case EStatus::UNAUTHORIZED: return "UNAUTHORIZED";
        case EStatus::INTERNAL_ERROR: return "INTERNAL_ERROR";
        case EStatus::ABORTED: return "ABORTED";
        case EStatus::UNAVAILABLE: return "UNAVAILABLE";
        case EStatus::OVERLOADED: return "OVERLOADED";
        case EStatus::SCHEME_ERROR: return "SCHEME_ERROR";
        case EStatus::GENERIC_ERROR: return "GENERIC_ERROR";
        case EStatus::TIMEOUT: return "TIMEOUT";
        case EStatus::BAD_SESSION: return "BAD_SESSION";
        case EStatus::PRECONDITION_FAILED: return "PRECONDITION_FAILED";
        case EStatus::ALREADY_EXISTS: return "ALREADY_EXISTS";
        case EStatus::NOT_FOUND: return "NOT_FOUND";
        case EStatus::SESSION_EXPIRED: return "SESSION_EXPIRED";
        case EStatus::CANCELLED: return "CANCELLED";
        case EStatus::UNDETERMINED: return "UNDETERMINED";
        case EStatus::UNSUPPORTED: return "UNSUPPORTED";
        case EStatus::SESSION_BUSY: return "SESSION_BUSY";
        case EStatus::EXTERNAL_ERROR: return "EXTERNAL_ERROR";
        case EStatus::TRANSPORT_UNAVAILABLE: return "TRANSPORT_UNAVAILABLE";
        case EStatus::CLIENT_RESOURCE_EXHAUSTED: return "CLIENT_RESOURCE_EXHAUSTED";
        case EStatus::CLIENT_DEADLINE_EXCEEDED: return "CLIENT_DEADLINE_EXCEEDED";
        case EStatus::CLIENT_INTERNAL_ERROR: return "CLIENT_INTERNAL_ERROR";
        case EStatus::CLIENT_CANCELLED: return "CLIENT_CANCELLED";
        case EStatus::CLIENT_UNAUTHENTICATED: return "CLIENT_UNAUTHENTICATED";
        case EStatus::CLIENT_CALL_UNIMPLEMENTED: return "CLIENT_CALL_UNIMPLEMENTED";
        case EStatus::CLIENT_OUT_OF_RANGE: return "CLIENT_OUT_OF_RANGE";
        case EStatus::CLIENT_LIMITS_REACHED: return "CLIENT_LIMITS_REACHED";
        default: return "UNKNOWN_" + std::to_string(static_cast<size_t>(status));
    }
}

} // namespace

TQuerySpan::TQuerySpan(std::shared_ptr<NMetrics::ITracer> tracer, const std::string& operationName, const std::string& endpoint) {
    if (!tracer) return;

    std::string host;
    int port;
    ParseEndpoint(endpoint, host, port);

    Span_ = tracer->StartSpan("ydb." + operationName, NMetrics::ESpanKind::CLIENT);
    Span_->SetAttribute("db.system.name", "ydb");
    Span_->SetAttribute("server.address", host);
    Span_->SetAttribute("server.port", static_cast<int64_t>(port));
}

TQuerySpan::~TQuerySpan() {
    if (Span_) {
        Span_->End();
    }
}

void TQuerySpan::End(EStatus status) {
    if (Span_) {
        Span_->SetAttribute("db.response.status_code", static_cast<int64_t>(status));
        if (status != EStatus::SUCCESS) {
            Span_->SetAttribute("error.type", StatusToString(status));
        }
        Span_->End();
        Span_.reset();
    }
}

} // namespace NYdb::NQuery
