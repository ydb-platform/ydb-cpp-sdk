#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/yson/public.h
#include <ydb-cpp-sdk/library/yt/misc/enum.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>

#include <ydb-cpp-sdk/library/yt/yson_string/public.h>
#include <ydb-cpp-sdk/library/yt/yson/public.h>
========
#include <src/library/yt/misc/enum.h>
#include <src/util/generic/yexception.h>

#include <src/library/yt/yson_string/public.h>
#include <src/library/yt/yson/public.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/yson/public.h

namespace NYson {

    ////////////////////////////////////////////////////////////////////////////////

    using NYT::NYson::EYsonFormat;
    using NYT::NYson::EYsonType;

    class TYsonStringBuf;

    struct TYsonConsumerBase;

    class TYsonWriter;
    class TYsonParser;
    class TStatelessYsonParser;
    class TYsonListParser;

    class TYsonException
       : public yexception {};

    ////////////////////////////////////////////////////////////////////////////////

} // namespace NYson
