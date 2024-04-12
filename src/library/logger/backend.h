#pragma once

#include "priority.h"

<<<<<<<< HEAD:include/ydb-cpp-sdk/library/logger/backend.h
#include <ydb-cpp-sdk/util/generic/noncopyable.h>
========
#include <src/util/generic/noncopyable.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/logger/backend.h
#include <vector>
#include <cstddef>

struct TLogRecord;

// NOTE: be aware that all `TLogBackend`s are registred in singleton.
class TLogBackend: public TNonCopyable {
public:
    TLogBackend() noexcept;
    virtual ~TLogBackend();

    virtual void WriteData(const TLogRecord& rec) = 0;
    virtual void ReopenLog() = 0;

    // Does not guarantee consistency with previous WriteData() calls:
    // log entries could be written to the new (reopened) log file due to
    // buffering effects.
    virtual void ReopenLogNoFlush();

    virtual ELogPriority FiltrationLevel() const;

    static void ReopenAllBackends(bool flush = true);

    virtual size_t QueueSize() const;
};
