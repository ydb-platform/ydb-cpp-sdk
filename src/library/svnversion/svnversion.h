#pragma once

#if !defined(FROM_IMPL)
#define PROGRAM_VERSION GetProgramSvnVersion()
#define ARCADIA_SOURCE_PATH GetArcadiaSourcePath()
#define PRINT_VERSION PrintSvnVersionAndExit(argc, (char**)argv)
#define PRINT_VERSION_EX(opts) PrintSvnVersionAndExitEx(argc, (char**)argv, opts)
#endif

#include <ydb-cpp-sdk/util/system/compiler.h>

// Automatically generated functions.
#include "scripts/c_templates/svnversion.h"
