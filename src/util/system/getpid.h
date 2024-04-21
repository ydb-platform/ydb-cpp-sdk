#pragma once

#include <ydb-cpp-sdk/util/system/platform.h>
#include <ydb-cpp-sdk/util/system/types.h>

#if defined(_win_)
using TProcessId = ui32; // DWORD
#else
using TProcessId = pid_t;
#endif

TProcessId GetPID();
