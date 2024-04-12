#pragma once

/*
 * WARN:
 * including this header does not make a lot of sense.
 * You should just #include all necessary headers from Windows SDK,
 * and then #include <src/util/system/win_undef.h> in order to undefine some common macros.
 */

<<<<<<<< HEAD:include/ydb-cpp-sdk/util/system/winint.h
#include <ydb-cpp-sdk/util/system/platform.h>
========
#include <src/util/system/platform.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/system/winint.h

#if defined(_win_)
    #include <windows.h>
#endif

<<<<<<<< HEAD:include/ydb-cpp-sdk/util/system/winint.h
#include <ydb-cpp-sdk/util/system/win_undef.h>
========
#include <src/util/system/win_undef.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/system/winint.h
