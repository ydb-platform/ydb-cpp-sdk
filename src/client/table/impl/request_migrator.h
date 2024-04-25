#pragma once

#include "client_session.h"

<<<<<<< HEAD
<<<<<<<< HEAD:src/client/table/impl/request_migrator.h
#include <ydb-cpp-sdk/client/table/table.h>

#include <ydb-cpp-sdk/library/threading/future/future.h>
========
#include <src/client/ydb_table/table.h>

#include <src/library/threading/future/future.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/impl/request_migrator.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_table/impl/request_migrator.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/client/table/table.h>

#include <ydb-cpp-sdk/library/threading/future/future.h>
>>>>>>> origin/main

#include <string>

#include <mutex>
#include <memory>

namespace NYdb {

namespace NMath {

struct TStats {
    const ui64 Cv;
    const float Mean;
};

TStats CalcCV(const std::vector<size_t>&);

} // namespace NMath

namespace NTable {

// TableClientImpl interface for migrator
// Migrator should be able
// - PrepareDataQuery
// - Schedule some cb to be executed
class IMigratorClient {
public:
    virtual ~IMigratorClient() = default;
    virtual void ScheduleTaskUnsafe(std::function<void()>&& fn, TDuration timeout) = 0;
};

class TRequestMigrator {
public:
    // Set host to migrate session from.
    // If the target session set requests will be reprepared on the new session in background.
    // Real migration will be performed after DoCheckAndMigrate call
    // IMPORTANT: SetHost methods are not reentrant. Moreover new call of this methods allowed only if
    // future returned from previous call was set.
    // The value of future means number of requests migrated
    void SetHost(ui64 nodeId);

    // Checks and perform migration if the session suitable to be removed from host.
    // Prepared requests will be migrated if target session was set along with host.
    // Returns false if session is not suitable or unable to get lock to start migration
    // Returns true if session is suitable in this case Unlink methos on the session is called
    // This methos is thread safe.
    bool DoCheckAndMigrate(const TKqpSessionCommon* session);

    // Reset migrator to initiall state if migration was not started and returns true
    // Returns false if migration was started
    bool Reset();
private:
    bool IsOurSession(const TKqpSessionCommon* session) const;

    ui64 CurHost_ = 0;

    mutable std::mutex Lock_;
};

} // namespace NTable
} // namespace NYdb
