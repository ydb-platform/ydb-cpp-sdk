#include "file_lock.h"
#include <ydb-cpp-sdk/util/system/flock.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/yexception.h>
=======
#include <src/util/generic/yexception.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/yexception.h>
>>>>>>> origin/main

#include <cerrno>

namespace {
    int GetMode(const EFileLockType type) {
        switch (type) {
            case EFileLockType::Exclusive:
                return LOCK_EX;
            case EFileLockType::Shared:
                return LOCK_SH;
            default:
                Y_UNREACHABLE();
        }
        Y_UNREACHABLE();
    }
}

TFileLock::TFileLock(const std::string& filename, const EFileLockType type)
    : TFile(filename, OpenAlways | RdOnly)
    , Type(type)
{
}

void TFileLock::Acquire() {
    Flock(GetMode(Type));
}

bool TFileLock::TryAcquire() {
    try {
        Flock(GetMode(Type) | LOCK_NB);
        return true;
    } catch (const TSystemError& e) {
        if (e.Status() != EWOULDBLOCK) {
            throw;
        }
        return false;
    }
}

void TFileLock::Release() {
    Flock(LOCK_UN);
}
