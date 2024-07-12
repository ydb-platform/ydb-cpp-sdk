#pragma once

#include <ydb-cpp-sdk/library/logger/backend.h>

#include <ydb-cpp-sdk/util/generic/fwd.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>

class TFileLogBackend: public TLogBackend {
public:
    TFileLogBackend(const std::string& path);
    ~TFileLogBackend() override;

    void WriteData(const TLogRecord& rec) override;
    void ReopenLog() override;

private:
    class TImpl;
    std::shared_ptr<TImpl> Impl_;
};
