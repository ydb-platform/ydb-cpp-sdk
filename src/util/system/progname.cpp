#include "execpath.h"
#include <ydb-cpp-sdk/util/system/progname.h>

#include <src/util/folder/dirut.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>

static const char* Argv0;

namespace {
    struct TProgramNameHolder {
        inline TProgramNameHolder()
            : ProgName(GetFileNameComponent(Argv0 ? Argv0 : GetExecPath().data()))
        {
        }

        std::string ProgName;
    };
}

const std::string& GetProgramName() {
    return Singleton<TProgramNameHolder>()->ProgName;
}

void SetProgramName(const char* argv0) {
    Argv0 = argv0;
}
