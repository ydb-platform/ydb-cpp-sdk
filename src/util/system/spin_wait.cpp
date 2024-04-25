#include <ydb-cpp-sdk/util/system/spin_wait.h>
#include "yield.h"
#include <ydb-cpp-sdk/util/system/compat.h>
#include <ydb-cpp-sdk/util/system/spinlock.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/digest/numeric.h>
#include <ydb-cpp-sdk/util/generic/utility.h>
=======
#include <src/util/digest/numeric.h>
#include <src/util/generic/utility.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/digest/numeric.h>
#include <ydb-cpp-sdk/util/generic/utility.h>
>>>>>>> origin/main

#include <atomic>

namespace {
    unsigned RandomizeSleepTime(unsigned t) noexcept {
        static std::atomic<unsigned> counter = 0;
        const unsigned rndNum = IntHash(++counter);

        return (t * 4 + (rndNum % t) * 2) / 5;
    }

    //arbitrary values
    constexpr unsigned MIN_SLEEP_TIME = 500;
    constexpr unsigned MAX_SPIN_COUNT = 0x7FF;
}

TSpinWait::TSpinWait() noexcept
    : T(MIN_SLEEP_TIME)
    , C(0)
{
}

void TSpinWait::Sleep() noexcept {
    ++C;

    if (C == MAX_SPIN_COUNT) {
        ThreadYield();
    } else if ((C & MAX_SPIN_COUNT) == 0) {
        usleep(RandomizeSleepTime(T));

        T = Min<unsigned>(T * 3 / 2, 20000);
    } else {
        SpinLockPause();
    }
}
