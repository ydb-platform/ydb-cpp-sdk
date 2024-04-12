#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/datetime/base.h>
=======
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NRetryPrivate {
    TDuration AddRandomDelta(TDuration delta);
    TDuration AddIncrement(ui32 attempt, TDuration increment);
    TDuration AddExponentialMultiplier(ui32 attempt, TDuration exponentialMultiplier);

}
