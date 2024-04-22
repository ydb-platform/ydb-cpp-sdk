#include "null.h"
#include <ydb-cpp-sdk/util/stream/debug.h>

#include <ydb-cpp-sdk/util/string/cast.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>

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
