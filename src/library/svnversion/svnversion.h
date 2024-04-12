#pragma once

#if !defined(FROM_IMPL)
#define PROGRAM_VERSION GetProgramSvnVersion()
#define ARCADIA_SOURCE_PATH GetArcadiaSourcePath()
#define PRINT_VERSION PrintSvnVersionAndExit(argc, (char**)argv)
#define PRINT_VERSION_EX(opts) PrintSvnVersionAndExitEx(argc, (char**)argv, opts)
#endif

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/compiler.h>
=======
#include <src/util/system/compiler.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

// Automatically generated functions.
#include "scripts/c_templates/svnversion.h"
