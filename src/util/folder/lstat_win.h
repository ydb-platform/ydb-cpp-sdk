#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/defaults.h>
=======
#include <src/util/system/defaults.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/defaults.h>
>>>>>>> origin/main
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
