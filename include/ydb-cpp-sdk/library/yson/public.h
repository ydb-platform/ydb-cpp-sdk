#pragma once

#include <ydb-cpp-sdk/library/yt/misc/enum.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>

#include <ydb-cpp-sdk/library/yt/yson_string/public.h>
#include <ydb-cpp-sdk/library/yt/yson/public.h>

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
