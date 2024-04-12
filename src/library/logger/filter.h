#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/logger/priority.h>
#include <ydb-cpp-sdk/library/logger/record.h>
#include <ydb-cpp-sdk/library/logger/backend.h>

#include <ydb-cpp-sdk/util/generic/ptr.h>
=======
#include "priority.h"
#include "record.h"
#include "backend.h"
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

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
