#pragma once

#include <ydb-cpp-sdk/util/datetime/base.h>

namespace NRetryPrivate {
    TDuration AddRandomDelta(TDuration delta);
    TDuration AddIncrement(ui32 attempt, TDuration increment);
    TDuration AddExponentialMultiplier(ui32 attempt, TDuration exponentialMultiplier);

}
