#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/defaults.h>
#include <ydb-cpp-sdk/util/generic/utility.h>
#include <ydb-cpp-sdk/util/datetime/base.h>
=======
#include "defaults.h"
#include <src/util/generic/utility.h>
#include <src/util/datetime/base.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

/// portable getrusage

struct TRusage {
    // some fields may be zero if unsupported

    ui64 MaxRss = 0;
    ui64 MajorPageFaults = 0;
    TDuration Utime;
    TDuration Stime;

    void Fill();

    static size_t GetCurrentRSS();

    static TRusage Get() {
        TRusage r;
        r.Fill();
        return r;
    }
};
