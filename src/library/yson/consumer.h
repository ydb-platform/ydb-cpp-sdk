#pragma once

#include <src/library/yt/yson/consumer.h>

#include <ydb-cpp-sdk/util/system/defaults.h>

namespace NYson {
    struct TYsonConsumerBase
       : public virtual NYT::NYson::IYsonConsumer {
        void OnRaw(std::string_view ysonNode, NYT::NYson::EYsonType type) override;
    };
} // namespace NYson
