#pragma once

#include <ydb-cpp-sdk/client/types/status_codes.h>

#include <src/library/retry/retry.h>

namespace NYdb::NPersQueue {
    ERetryErrorClass GetRetryErrorClass(EStatus status);
    ERetryErrorClass GetRetryErrorClassV2(EStatus status);
}
