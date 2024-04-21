#pragma once

#include <functional>
#include <ydb-cpp-sdk/util/datetime/base.h>

namespace NYdb {

class IClientImplCommon {
public:
    virtual ~IClientImplCommon() = default;
    virtual void ScheduleTask(const std::function<void()>& fn, TDuration timeout) = 0;
};

}
