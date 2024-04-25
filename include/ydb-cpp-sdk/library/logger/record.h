#pragma once

#include "priority.h"

#include <string>
<<<<<<< HEAD
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/logger/record.h
#include <ydb-cpp-sdk/util/system/defaults.h>
========
#include <src/util/system/defaults.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/logger/record.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/logger/record.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/defaults.h>
>>>>>>> origin/main

#include <utility>
#include <vector>

struct TLogRecord {
    using TMetaFlags = std::vector<std::pair<std::string, std::string>>;

    const char* Data;
    size_t Len;
    ELogPriority Priority;
    TMetaFlags MetaFlags;

    inline TLogRecord(ELogPriority priority, const char* data, size_t len, TMetaFlags metaFlags = {}) noexcept
        : Data(data)
        , Len(len)
        , Priority(priority)
        , MetaFlags(std::move(metaFlags))
    {
    }
};
