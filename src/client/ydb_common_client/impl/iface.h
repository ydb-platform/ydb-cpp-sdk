#pragma once

#include <functional>
<<<<<<<< HEAD:src/client/common_client/impl/iface.h
#include <ydb-cpp-sdk/util/datetime/base.h>
========
#include <src/util/datetime/base.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_common_client/impl/iface.h

namespace NYdb {

class IClientImplCommon {
public:
    virtual ~IClientImplCommon() = default;
    virtual void ScheduleTask(const std::function<void()>& fn, TDuration timeout) = 0;
};

}
