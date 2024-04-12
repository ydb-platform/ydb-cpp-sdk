#include "init_atfork.h"
#include <ydb-cpp-sdk/util/random/random.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/system/yassert.h>
=======
#include <src/util/generic/singleton.h>
#include <src/util/system/yassert.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#if defined(_unix_)
    #include <pthread.h>
#endif

namespace {
    struct TInit {
        inline TInit() noexcept {
            (void)&AtFork;

#if defined(_unix_)
            Y_ABORT_UNLESS(pthread_atfork(nullptr, AtFork, nullptr) == 0, "it happens");
#endif
        }

        static void AtFork() noexcept {
            ResetRandomState();
        }
    };
}

void RNGInitAtForkHandlers() {
    SingletonWithPriority<TInit, 0>();
}
