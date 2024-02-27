#pragma once

#include <library/cpp/yt/yson/consumer.h>

#include <util/system/defaults.h>

namespace NYson {
    struct TYsonConsumerBase
       : public virtual NYT::NYson::IYsonConsumer {
        void OnRaw(std::string_view ysonNode, NYT::NYson::EYsonType type) override;
    };
} // namespace NYson
