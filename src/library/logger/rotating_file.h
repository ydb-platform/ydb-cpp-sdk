#pragma once

#include <ydb-cpp-sdk/library/logger/backend.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/fwd.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include <src/util/generic/fwd.h>
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/generic/fwd.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
>>>>>>> origin/main

class TRotatingFileLogBackend: public TLogBackend {
public:
    TRotatingFileLogBackend(const std::string& preRotatePath, const std::string& postRotatePath, const ui64 maxSizeBytes);
    TRotatingFileLogBackend(const std::string& path, const ui64 maxSizeBytes, const ui32 rotatedFilesCount);
    ~TRotatingFileLogBackend() override;

    void WriteData(const TLogRecord& rec) override;
    void ReopenLog() override;

private:
    class TImpl;
    TAtomicSharedPtr<TImpl> Impl_;
};
