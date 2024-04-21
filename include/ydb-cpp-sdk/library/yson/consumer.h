#pragma once

#include <ydb-cpp-sdk/library/yt/yson/consumer.h>

#include <ydb-cpp-sdk/util/system/defaults.h>

namespace NYson {
    struct TYsonConsumerBase
       : public virtual NYT::NYson::IYsonConsumer {
        void OnRaw(std::string_view ysonNode, NYT::NYson::EYsonType type) override;
    };
} // namespace NYson
