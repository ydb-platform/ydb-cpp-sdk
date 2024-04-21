#pragma once

#include <ydb-cpp-sdk/library/logger/backend.h>

#include <src/util/generic/fwd.h>
#include <src/util/generic/ptr.h>

class TFileLogBackend: public TLogBackend {
public:
    TFileLogBackend(const std::string& path);
    ~TFileLogBackend() override;

    void WriteData(const TLogRecord& rec) override;
    void ReopenLog() override;

private:
    class TImpl;
    TAtomicSharedPtr<TImpl> Impl_;
};
