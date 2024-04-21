#include "last_getopt.h"
#include "last_getopt_support.h"
#include "modchooser.h"
#include "opt.h"
#include "opt2.h"
#include "posix_getopt.h"
#include "ygetopt.h"

#include <src/library/svnversion/svnversion.h>
#include <src/library/build_info/build_info.h>

namespace NLastGetoptPrivate {
    std::string InitVersionString() {
        std::string ts = GetProgramSvnVersion();
        ts += "\n";
        ts += GetBuildInfo();
        std::string sandboxTaskId = GetSandboxTaskId();
        if (sandboxTaskId != std::string("0")) {
            ts += "\nSandbox task id: ";
            ts += sandboxTaskId;
        }
        return ts;
    }

    std::string InitShortVersionString() {
        std::string ts = GetProgramShortVersionData();
        return ts;
    }

    std::string& VersionString();
    std::string& ShortVersionString();

    struct TInit {
        TInit() {
            VersionString() = InitVersionString();
            ShortVersionString() = InitShortVersionString();
        }
    } Init;

}
