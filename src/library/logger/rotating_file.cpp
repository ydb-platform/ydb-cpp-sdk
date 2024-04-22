#include "rotating_file.h"
#include "file.h"
#include <ydb-cpp-sdk/library/logger/record.h>

#include <ydb-cpp-sdk/util/string/builder.h>

#include <ydb-cpp-sdk/util/string/builder.h>
#include <src/util/system/fstat.h>
#include <ydb-cpp-sdk/util/system/rwlock.h>
#include <src/util/system/fs.h>
#include <ydb-cpp-sdk/library/deprecated/atomic/atomic.h>

/*
 * rotating file log
 * if Size_ > MaxSizeBytes
 *    Path.(N-1) -> Path.N
 *    Path.(N-2) -> Path.(N-1)
 *    ...
 *    Path.1     -> Path.2
 *    Path       -> Path.1
 */
class TRotatingFileLogBackend::TImpl {
public:
    inline TImpl(const std::string& path, const ui64 maxSizeBytes, const ui32 rotatedFilesCount)
        : Log_(path)
        , Path_(path)
        , MaxSizeBytes_(maxSizeBytes)
        , Size_(TFileStat(Path_.c_str()).Size)
        , RotatedFilesCount_(rotatedFilesCount)
    {
        Y_ENSURE(RotatedFilesCount_ != 0);
    }

    inline void WriteData(const TLogRecord& rec) {
        if (static_cast<ui64>(AtomicGet(Size_)) > MaxSizeBytes_) {
            TWriteGuard guard(Lock_);
            if (static_cast<ui64>(AtomicGet(Size_)) > MaxSizeBytes_) {
                std::string newLogPath(TStringBuilder() << Path_ << "." << RotatedFilesCount_);
                for (size_t fileId = RotatedFilesCount_ - 1; fileId; --fileId) {
                    std::string oldLogPath(TStringBuilder() << Path_ << "." << fileId);
                    NFs::Rename(oldLogPath.c_str(), newLogPath.c_str());
                    newLogPath = oldLogPath;
                }
                NFs::Rename(Path_.c_str(), newLogPath.c_str());
                Log_.ReopenLog();
                AtomicSet(Size_, 0);
            }
        }
        TReadGuard guard(Lock_);
        Log_.WriteData(rec);
        AtomicAdd(Size_, rec.Len);
    }

    inline void ReopenLog() {
        TWriteGuard guard(Lock_);

        Log_.ReopenLog();
        AtomicSet(Size_, TFileStat(Path_.c_str()).Size);
    }

private:
    TRWMutex Lock_;
    TFileLogBackend Log_;
    const std::string Path_;
    const ui64 MaxSizeBytes_;
    TAtomic Size_;
    const ui32 RotatedFilesCount_;
};

TRotatingFileLogBackend::TRotatingFileLogBackend(const std::string& path, const ui64 maxSizeByte, const ui32 rotatedFilesCount)
    : Impl_(new TImpl(path, maxSizeByte, rotatedFilesCount))
{
}

TRotatingFileLogBackend::~TRotatingFileLogBackend() {
}

void TRotatingFileLogBackend::WriteData(const TLogRecord& rec) {
    Impl_->WriteData(rec);
}

void TRotatingFileLogBackend::ReopenLog() {
    TAtomicSharedPtr<TImpl> copy = Impl_;
    if (copy) {
        copy->ReopenLog();
    }
}
