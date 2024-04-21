#pragma once

#include <src/util/generic/fwd.h>

void SetProgramName(const char* argv0);

#define SAVE_PROGRAM_NAME        \
    do {                         \
        SetProgramName(argv[0]); \
    } while (0)

/// guaranted return the same immutable instance of std::string
const std::string& GetProgramName();
