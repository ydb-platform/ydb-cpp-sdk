#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define FROM_IMPL
#include "svnversion.h"

#include <library/cpp/string_utils/misc/misc.h>
#include <string_view>

extern "C" void PrintProgramSvnVersion() {
    puts(GetProgramSvnVersion());
}

extern "C" void PrintSvnVersionAndExit0() {
    PrintProgramSvnVersion();
    exit(0);
}

extern "C" void PrintSvnVersionAndExitEx(int argc, char* argv[], const char* opts) {
    if (2 == argc) {
        for (std::string_view all = opts, versionOpt; NUtils::TrySplit(all, versionOpt, all, ';');) {
            if (versionOpt == argv[1]) {
                PrintSvnVersionAndExit0();
            }
        }
    }
}

extern "C" void PrintSvnVersionAndExit(int argc, char* argv[]) {
    PrintSvnVersionAndExitEx(argc, argv, "--version");
}

#undef FROM_IMPL
