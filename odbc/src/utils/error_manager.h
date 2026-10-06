#pragma once

#include "odbc_compat.h"
#include "handle.h"
#include <atomic>
#include <functional>
#include <vector>
#include <string>
#include <exception>
#include <mutex>
#include <shared_mutex>
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
    virtual ~TErrorManager() = default;
    virtual void BeforeCall() {}
    void SetParent(std::shared_ptr<TErrorManager> parent) { Parent_ = std::move(parent); }
    void SetLifecycle(std::shared_mutex* lifecycle) { Lifecycle_ = lifecycle; }
    std::shared_mutex* GetLifecycle() const { return Lifecycle_; }
    bool IsRetired() const { return Retired_.load(std::memory_order_relaxed); }
    void Retire() { Retired_.store(true, std::memory_order_relaxed); }
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
    std::shared_ptr<TErrorManager> Parent_;
    std::shared_mutex* Lifecycle_ = nullptr;
    std::atomic<bool> Retired_ = false;
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

template <ECallMode Mode, typename Handle, bool Exclusive = false, class Fn>
SQLRETURN CallOdbc(SQLHANDLE handlePtr, Fn&& func) {
    auto owner = std::dynamic_pointer_cast<Handle>(PinHandle(handlePtr));
    if (!owner) {
        return SQL_INVALID_HANDLE;
    }
    auto* handle = owner.get();
    std::shared_lock<std::shared_mutex> shared;
    std::unique_lock<std::shared_mutex> exclusive;
    if (auto* lifecycle = handle->GetLifecycle()) {
        if constexpr (Exclusive) {
            exclusive = std::unique_lock(*lifecycle);
        } else {
            shared = std::shared_lock(*lifecycle);
        }
    }
    std::unique_lock lock(handle->GetMutex());
    if (handle->IsRetired()) {
        return SQL_INVALID_HANDLE;
    }
    if constexpr (Mode != ECallMode::Diagnostic) {
        handle->ClearErrors();
    }
    try {
        if constexpr (Mode == ECallMode::Ordinary) {
            handle->BeforeCall();
        }
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

} // namespace NYdb::NOdbc
