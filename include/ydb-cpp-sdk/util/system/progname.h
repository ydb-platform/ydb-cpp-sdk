#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/util/system/progname.h
#include <ydb-cpp-sdk/util/generic/fwd.h>
========
#include <src/util/generic/fwd.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/system/progname.h

void SetProgramName(const char* argv0);

#define SAVE_PROGRAM_NAME        \
    do {                         \
        SetProgramName(argv[0]); \
    } while (0)

/// guaranted return the same immutable instance of std::string
const std::string& GetProgramName();
