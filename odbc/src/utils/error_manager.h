#pragma once

#include "odbc_compat.h"
#include <functional>
#include <vector>
#include <string>
#include <exception>
#include <mutex>
#include <type_traits>
#include <utility>

#include <ydb-cpp-sdk/client/types/status/status.h>

namespace NYdb::NOdbc {

struct TErrorInfo {
    std::string SqlState;
    SQLINTEGER NativeError;
    std::string Message;
};

class TOdbcException : public std::exception {
public:
    TOdbcException(const std::string& sqlState, SQLINTEGER nativeError,
                    const std::string& message, SQLRETURN returnCode = SQL_ERROR)
        : SqlState_(sqlState)
        , NativeError_(nativeError)
        , Message_(message)
        , ReturnCode_(returnCode)
    {}

    const std::string& GetSqlState() const {
        return SqlState_;
    }

    SQLINTEGER GetNativeError() const {
        return NativeError_;
    }

    const std::string& GetMessage() const {
        return Message_;
    }

    SQLRETURN GetReturnCode() const {
        return ReturnCode_;
    }

    const char* what() const noexcept override {
        return Message_.c_str();
    }

private:
    std::string SqlState_;
    SQLINTEGER NativeError_;
    std::string Message_;
    SQLRETURN ReturnCode_;
};

class TErrorManager {
public:
    SQLRETURN AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message, SQLRETURN returnCode = SQL_ERROR);
    SQLRETURN AddError(const TOdbcException& ex);
    SQLRETURN AddError(const TStatus& status);

    void ClearErrors();
    std::recursive_mutex& GetMutex() const noexcept {
        return Mutex_;
    }

    void SetLastReturnCode(SQLRETURN code) {
        LastReturnCode_ = code;
    }

    SQLRETURN GetDiagRec(SQLSMALLINT recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeError, 
                        SQLCHAR* messageText, SQLSMALLINT bufferLength, SQLSMALLINT* textLength);
    virtual SQLRETURN GetDiagField(SQLSMALLINT recNumber, SQLSMALLINT diagIdentifier,
                           SQLPOINTER diagInfoPtr, SQLSMALLINT bufferLength, SQLSMALLINT* stringLengthPtr);

private:
    mutable std::recursive_mutex Mutex_;
    std::vector<TErrorInfo> Errors_;
    SQLRETURN LastReturnCode_ = SQL_SUCCESS;
};

enum class ECallMode : unsigned char { Ordinary, Diagnostic, Consuming };

SQLRETURN RecordCurrentException(TErrorManager& errors);

template <class Fn, class... Args>
SQLRETURN InvokeOdbc(Fn&& fn, Args&&... args) {
    if constexpr (std::is_void_v<std::invoke_result_t<Fn, Args...>>) {
        std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...);
        return SQL_SUCCESS;
    } else {
        return std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...);
    }
}

template <ECallMode Mode, typename Handle, class Fn>
SQLRETURN CallOdbc(SQLHANDLE handlePtr, Fn&& func) {
    if (!handlePtr) {
        return SQL_INVALID_HANDLE;
    }
    auto* handle = static_cast<Handle*>(handlePtr);
    std::unique_lock lock(handle->GetMutex(), std::defer_lock);
    if constexpr (Mode != ECallMode::Consuming) {
        lock.lock();
    }
    if constexpr (Mode != ECallMode::Diagnostic) {
        handle->ClearErrors();
    }
    try {
        const SQLRETURN ret = InvokeOdbc(std::forward<Fn>(func), handle);
        if constexpr (Mode == ECallMode::Ordinary) {
            handle->SetLastReturnCode(ret);
        }
        return ret;
    } catch (...) {
        if constexpr (Mode == ECallMode::Diagnostic) {
            return SQL_ERROR;
        }
        return RecordCurrentException(*handle);
    }
}

enum class ENullInputHandlePolicy : unsigned char { Reject, Allow };

template <class Fn>
SQLRETURN CallOdbcUnchecked(
    SQLHANDLE handlePtr,
    Fn&& func,
    ENullInputHandlePolicy nullInputPolicy = ENullInputHandlePolicy::Reject) {
    if (!handlePtr && nullInputPolicy == ENullInputHandlePolicy::Reject) {
        return SQL_INVALID_HANDLE;
    }
    try {
        const SQLRETURN ret = InvokeOdbc(std::forward<Fn>(func));
        if (handlePtr) {
            static_cast<TErrorManager*>(handlePtr)->SetLastReturnCode(ret);
        }
        return ret;
    } catch (...) {
        if (handlePtr) {
            static_cast<TErrorManager*>(handlePtr)->SetLastReturnCode(SQL_ERROR);
        }
        return SQL_ERROR;
    }
}

} // namespace NYdb::NOdbc
