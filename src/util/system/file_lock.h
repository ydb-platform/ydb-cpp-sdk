#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/fwd.h>
#include <ydb-cpp-sdk/util/generic/noncopyable.h>
#include <ydb-cpp-sdk/util/system/file.h>
=======
#include <src/util/generic/fwd.h>
#include <src/util/generic/noncopyable.h>
#include <src/util/system/file.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/fwd.h>
#include <ydb-cpp-sdk/util/generic/noncopyable.h>
#include <ydb-cpp-sdk/util/system/file.h>
>>>>>>> origin/main

enum class EFileLockType {
    Exclusive,
    Shared
};

class TFileLock: public TFile {
public:
    TFileLock(const std::string& filename, const EFileLockType type = EFileLockType::Exclusive);

    void Acquire();
    bool TryAcquire();
    void Release();

    inline void lock() {
        Acquire();
    }

    inline bool try_lock() {
        return TryAcquire();
    }

    inline void unlock() {
        Release();
    }

private:
    EFileLockType Type;
};
