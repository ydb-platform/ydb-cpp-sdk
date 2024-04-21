#pragma once

#include <ydb-cpp-sdk/library/logger/log.h>
#include <ydb-cpp-sdk/library/logger/backend.h>

#include <src/util/generic/fwd.h>
#include <src/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/generic/size_literals.h>

#include <atomic>

class TReopenLogBackend: public TLogBackend {
public:
    explicit TReopenLogBackend(std::unique_ptr<TLogBackend>&& backend, ui64 bytesWrittenLimit = 1_GB)
        : Backend_(std::move(backend)), BytesWrittenLimit_(bytesWrittenLimit), BytesWritten_(0) {
            Y_ENSURE(BytesWrittenLimit_ > 0);
    }

    void WriteData(const TLogRecord& rec) override {
        const ui64 prevWritten = BytesWritten_.fetch_add(rec.Len);
        if (prevWritten < BytesWrittenLimit_ && prevWritten + rec.Len >= BytesWrittenLimit_) {
            try {
                ReopenLog();
            } catch (...) {
            }
        }
        Backend_->WriteData(rec);
    }

    void ReopenLog() override {
        BytesWritten_.store(0);
        Backend_->ReopenLog();
    }

private:
    const std::unique_ptr<TLogBackend> Backend_;

    const ui64 BytesWrittenLimit_;
    std::atomic<ui64> BytesWritten_;
};
