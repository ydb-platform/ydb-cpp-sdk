#pragma once

#include "error_manager.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <utility>

namespace NYdb::NOdbc {

enum class ECallMode : unsigned char { Ordinary, Diagnostic, Consuming };

class THandle : public TErrorManager {
public:
    virtual void BeforeCall() {}
    void SetParent(std::shared_ptr<THandle> parent) { Parent_ = std::move(parent); }
    void SetLifecycle(std::shared_mutex* lifecycle) { Lifecycle_ = lifecycle; }
    std::shared_mutex* GetLifecycle() const { return Lifecycle_; }
    bool IsRetired() const { return Retired_.load(std::memory_order_relaxed); }
    void Retire() { Retired_.store(true, std::memory_order_relaxed); }
    std::unique_lock<std::mutex> LockOperation() const { return std::unique_lock(OperationMutex_); }

protected:
    // API calls hold the connection lifecycle gate before this lock. It protects
    // this handle's mutable state and diagnostics; SDK objects need no extra lock.
    mutable std::mutex OperationMutex_;

private:
    std::shared_ptr<THandle> Parent_;
    std::shared_mutex* Lifecycle_ = nullptr;
    std::atomic<bool> Retired_ = false;
};

void RegisterHandle(SQLHANDLE handle, std::shared_ptr<THandle> owner);
std::shared_ptr<THandle> PinHandle(SQLHANDLE handle);
void UnregisterHandle(SQLHANDLE handle);

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
    auto lock = handle->LockOperation();
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
