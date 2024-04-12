#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/defaults.h>
=======
#include <src/util/system/defaults.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
#include "fts.h"

#ifdef _win_
    #include <sys/stat.h>

    #ifdef __cplusplus
extern "C" {
    #endif

    #define _S_IFLNK 0xA000
    int lstat(const char* fileName, stat_struct* fileStat);

    #ifdef __cplusplus
}
    #endif

#endif //_win_
