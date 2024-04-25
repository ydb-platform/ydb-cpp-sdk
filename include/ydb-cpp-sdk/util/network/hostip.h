#pragma once

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/network/hostip.h
#include <ydb-cpp-sdk/util/system/defaults.h>
========
#include <src/util/system/defaults.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/network/hostip.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/network/hostip.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/defaults.h>
>>>>>>> origin/main

namespace NResolver {
    // resolve hostname and fills up to *slots slots in ip array;
    // actual number of slots filled is returned in *slots;
    int GetHostIP(const char* hostname, ui32* ip, size_t* slots);
    int GetDnsError();

    inline int GetHostIP(const char* hostname, ui32* ip) {
        size_t slots = 1;

        return GetHostIP(hostname, ip, &slots);
    }
}
