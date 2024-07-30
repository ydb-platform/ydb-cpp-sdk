#pragma once

#include <ydb-cpp-sdk/client/types/status/status.h>
#include <ydb-cpp-sdk/client/types/status_codes.h>

#include <src/library/retry/retry_policy.h>

namespace NYdb::NTopic {

ERetryErrorClass GetRetryErrorClass(EStatus status);
ERetryErrorClass GetRetryErrorClassV2(EStatus status);

}  // namespace NYdb::NTopic
