#pragma once

#include "priority.h"
#include "record.h"


<<<<<<<< HEAD:include/ydb-cpp-sdk/library/logger/element.h
#include <ydb-cpp-sdk/util/string/cast.h>
#include <ydb-cpp-sdk/util/stream/tempbuf.h>
========
#include <src/util/string/cast.h>
#include <src/util/stream/tempbuf.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/logger/element.h


class TLog;

/// @warning Better don't use directly.
class TLogElement: public TTempBufOutput {
public:
    explicit TLogElement(const TLog* parent);
    explicit TLogElement(const TLog* parent, ELogPriority priority);

    TLogElement(TLogElement&&) noexcept = default;
    TLogElement& operator=(TLogElement&&) noexcept = default;

    ~TLogElement() override;

    template <class T>
    inline TLogElement& operator<<(const T& t) {
        static_cast<IOutputStream&>(*this) << t;

        return *this;
    }

    /// @note For pretty usage: logger << TLOG_ERROR << "Error description";
    inline TLogElement& operator<<(ELogPriority priority) {
        Flush();
        Priority_ = priority;
        return *this;
    }

    template<typename T>
    TLogElement& With(const std::string_view key, const T value) {
        Context_.emplace_back(key, ToString(value));

        return *this;
    }

    ELogPriority Priority() const noexcept {
        return Priority_;
    }

protected:
    void DoFlush() override;

protected:
    const TLog* Parent_ = nullptr;
    ELogPriority Priority_;
    TLogRecord::TMetaFlags Context_;
};
