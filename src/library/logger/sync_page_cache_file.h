#pragma once

#include <ydb-cpp-sdk/library/logger/backend.h>

#include <ydb-cpp-sdk/util/generic/fwd.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>

#include <memory>

class TSyncPageCacheFileLogBackend final: public TLogBackend {
public:
    TSyncPageCacheFileLogBackend(const std::string& path, size_t maxBufferSize, size_t maxPendingCacheSize);
    ~TSyncPageCacheFileLogBackend();

    void WriteData(const TLogRecord& rec) override;
    void ReopenLog() override;

private:
    class TImpl;
    std::unique_ptr<TImpl> Impl_;
};
