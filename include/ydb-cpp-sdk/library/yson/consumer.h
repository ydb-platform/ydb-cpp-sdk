#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/yson/consumer.h
#include <ydb-cpp-sdk/library/yt/yson/consumer.h>

#include <ydb-cpp-sdk/util/system/defaults.h>
========
#include <src/library/yt/yson/consumer.h>

#include <src/util/system/defaults.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/yson/consumer.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/yson/consumer.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

namespace NYson {
    struct TYsonConsumerBase
       : public virtual NYT::NYson::IYsonConsumer {
        void OnRaw(std::string_view ysonNode, NYT::NYson::EYsonType type) override;
    };
} // namespace NYson
