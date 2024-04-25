#pragma once

<<<<<<< HEAD
<<<<<<<< HEAD:src/client/types/core_facility/core_facility.h
#include <ydb-cpp-sdk/client/types/status_codes.h>
#include <ydb-cpp-sdk/client/types/status/status.h>
========
#include <src/client/ydb_types/status_codes.h>
#include <src/client/ydb_types/status/status.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_types/core_facility/core_facility.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_types/core_facility/core_facility.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/client/types/status_codes.h>
#include <ydb-cpp-sdk/client/types/status/status.h>
>>>>>>> origin/main

namespace NYdb {
using TPeriodicCb = std::function<bool(NYql::TIssues&&, EStatus)>;

// !!!Experimental!!!
// Allows to communicate with sdk core
class ICoreFacility {
public:
    virtual ~ICoreFacility() = default;
    // Add task to execute periodicaly
    // Task should return false to stop execution
    virtual void AddPeriodicTask(TPeriodicCb&& cb, TDuration period) = 0;
};

} // namespace NYdb
