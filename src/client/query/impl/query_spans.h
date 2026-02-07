#pragma once

#include <ydb-cpp-sdk/client/metrics/metrics.h>
#include <ydb-cpp-sdk/client/types/status_codes.h>
#include <ydb-cpp-sdk/client/types/status/status.h>

#include <memory>
#include <string>

namespace NYdb::inline V3::NQuery {

class TQuerySpan {
public:
    TQuerySpan(std::shared_ptr<NMetrics::ITracer> tracer, const std::string& operationName, const std::string& endpoint);
    ~TQuerySpan();

    void End(EStatus status);

private:
    std::shared_ptr<NMetrics::ISpan> Span_;
};

} // namespace NYdb::NQuery
