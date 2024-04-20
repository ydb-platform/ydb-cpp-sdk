#pragma once

#include "backend.h"

#include <util/generic/fwd.h>
#include <util/generic/ptr.h>

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
