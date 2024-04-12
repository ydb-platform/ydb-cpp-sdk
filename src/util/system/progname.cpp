#include "execpath.h"
#include <ydb-cpp-sdk/util/system/progname.h>

#include <src/util/folder/dirut.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/singleton.h>
=======
#include <src/util/generic/singleton.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

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
