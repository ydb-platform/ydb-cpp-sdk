#include "null.h"
#include <ydb-cpp-sdk/util/stream/debug.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/cast.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/string/cast.h>
#include <src/util/generic/singleton.h>
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/cast.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
>>>>>>> origin/main

#include <cstdio>
#include <cstdlib>

namespace {
    struct TDbgSelector {
        inline TDbgSelector() {
            char* dbg = std::getenv("DBGOUT");
            if (dbg) {
                Out = &std::cerr;
                try {
                    Level = FromString(dbg);
                } catch (const yexception&) {
                    Level = 0;
                }
            } else {
                Out = &std_cnull;
                Level = 0;
            }
        }

        std::ostream* Out;
        int Level;
    };
}

template <>
struct TSingletonTraits<TDbgSelector> {
    static constexpr size_t Priority = 8;
};

std::ostream& StdDbgStream() noexcept {
    return *(Singleton<TDbgSelector>()->Out);
}
