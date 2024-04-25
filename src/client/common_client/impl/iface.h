#pragma once

#include <functional>
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/common_client/impl/iface.h
#include <ydb-cpp-sdk/util/datetime/base.h>
========
#include <src/util/datetime/base.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_common_client/impl/iface.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_common_client/impl/iface.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/datetime/base.h>
>>>>>>> origin/main

namespace NYdb {

class IClientImplCommon {
public:
    virtual ~IClientImplCommon() = default;
    virtual void ScheduleTask(const std::function<void()>& fn, TDuration timeout) = 0;
};

}
