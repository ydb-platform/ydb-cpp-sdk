/*
 * The lack of #pragma once is intentional,
 * as the user might need to include this header multiple times.
 */

<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/util/system/win_undef.h
#include <ydb-cpp-sdk/util/system/platform.h>
========
#include <src/util/system/platform.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/system/win_undef.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/system/win_undef.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/platform.h>
>>>>>>> origin/main

#if defined(_win_)
    #undef GetFreeSpace
    #undef LoadImage
    #undef GetMessage
    #undef SendMessage
    #undef DeleteFile
    #undef GetUserName
    #undef CreateMutex
    #undef GetObject
    #undef GetGeoInfo
    #undef GetClassName
    #undef GetKValue
    #undef StartDoc
    #undef UpdateResource
    #undef GetNameInfo
    #undef GetProp
    #undef SetProp
    #undef RemoveProp

    // FIXME thegeorg@: undefining CONST breaks too many projects.
    // #undef CONST
    #undef DEFAULT_QUALITY
    #undef ERROR
    #undef IGNORE
    #undef OPTIONAL
    #undef TRANSPARENT

    #undef LANG_LAO
    #undef LANG_YI

    #undef CM_NONE

#endif
