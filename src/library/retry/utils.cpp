#include "utils.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/random/random.h>
=======
#include <src/util/random/random.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

TDuration NRetryPrivate::AddRandomDelta(TDuration maxDelta) {
    if (maxDelta == TDuration::Zero()) {
        return TDuration::Zero();
    }

    const TDuration delta = TDuration::MicroSeconds(RandomNumber(2 * maxDelta.MicroSeconds()));
    return delta - maxDelta;
}

TDuration NRetryPrivate::AddIncrement(ui32 attempt, TDuration increment) {
    return TDuration::MicroSeconds(attempt * increment.MicroSeconds());
}

TDuration NRetryPrivate::AddExponentialMultiplier(ui32 attempt, TDuration exponentialMultiplier) {
    return TDuration::MicroSeconds((1ull << Min(63u, attempt)) * exponentialMultiplier.MicroSeconds());
}
