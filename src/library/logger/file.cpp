#include "file.h"
#include <ydb-cpp-sdk/library/logger/record.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/system/file.h>
#include <ydb-cpp-sdk/util/system/rwlock.h>
=======
#include <src/util/system/file.h>
#include <src/util/system/rwlock.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/system/file.h>
#include <ydb-cpp-sdk/util/system/rwlock.h>
>>>>>>> origin/main

/*
 * file log
 */
class TFileLogBackend::TImpl {
public:
    inline TImpl(const std::string& path)
        : File_(OpenFile(path))
    {
    }

    inline void WriteData(const TLogRecord& rec) {
        //many writes are thread-safe
        TReadGuard guard(Lock_);

        File_.Write(rec.Data, rec.Len);
    }

    inline void ReopenLog() {
        //but log rotate not thread-safe
        TWriteGuard guard(Lock_);

        File_.LinkTo(OpenFile(File_.GetName()));
    }

private:
    static inline TFile OpenFile(const std::string& path) {
        return TFile(path.c_str(), OpenAlways | WrOnly | ForAppend | Seq | NoReuse);
    }

private:
    TRWMutex Lock_;
    TFile File_;
};

TFileLogBackend::TFileLogBackend(const std::string& path)
    : Impl_(new TImpl(path))
{
}

TFileLogBackend::~TFileLogBackend() {
}

void TFileLogBackend::WriteData(const TLogRecord& rec) {
    Impl_->WriteData(rec);
}

void TFileLogBackend::ReopenLog() {
    TAtomicSharedPtr<TImpl> copy = Impl_;
    if (copy) {
        copy->ReopenLog();
    }
}
