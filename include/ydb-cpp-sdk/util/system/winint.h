#pragma once

/*
 * WARN:
 * including this header does not make a lot of sense.
 * You should just #include all necessary headers from Windows SDK,
 * and then #include <src/util/system/win_undef.h> in order to undefine some common macros.
 */

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/system/winint.h
#include <ydb-cpp-sdk/util/system/platform.h>
========
#include <src/util/system/platform.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/system/winint.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/system/winint.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/platform.h>
>>>>>>> origin/main

#if defined(_win_)
    #include <windows.h>
#endif

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/system/winint.h
#include <ydb-cpp-sdk/util/system/win_undef.h>
========
#include <src/util/system/win_undef.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/system/winint.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/system/winint.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/win_undef.h>
>>>>>>> origin/main
