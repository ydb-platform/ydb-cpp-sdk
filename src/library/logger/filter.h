#pragma once

#include <ydb-cpp-sdk/library/logger/priority.h>
#include <ydb-cpp-sdk/library/logger/record.h>
#include <ydb-cpp-sdk/library/logger/backend.h>

#include <src/util/generic/ptr.h>

class TFilteredLogBackend: public TLogBackend {
    THolder<TLogBackend> Backend;
    ELogPriority Level;

public:
    TFilteredLogBackend(THolder<TLogBackend>&& t, ELogPriority level = LOG_MAX_PRIORITY) noexcept
        : Backend(std::move(t))
        , Level(level)
    {
    }

    ELogPriority FiltrationLevel() const override {
        return Level;
    }

    void ReopenLog() override {
        Backend->ReopenLog();
    }

    void WriteData(const TLogRecord& rec) override {
        if (rec.Priority <= (ELogPriority)Level) {
            Backend->WriteData(rec);
        }
    }
};
