#pragma once

#include <ydb-cpp-sdk/client/types/status_codes.h>

#include <string_view>

namespace NYdb::inline V3::NObservability {

// Maps EStatus to OTel-style error.type category:
// "transport_error" for client/transport-layer statuses (>= TRANSPORT_STATUSES_FIRST),
// "ydb_error"       for server-side YDB statuses.
std::string_view CategorizeErrorType(EStatus status) noexcept;

} // namespace NYdb::NObservability
