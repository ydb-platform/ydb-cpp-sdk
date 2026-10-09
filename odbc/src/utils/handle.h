#pragma once

#include "error_manager.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <utility>
#include <vector>

namespace NYdb::NOdbc {

class TEnvironment;

enum class ECallMode : unsigned char { Ordinary, Diagnostic, Consuming };

class THandle : public TErrorManager {
public:
    explicit THandle(std::shared_ptr<THandle> parent = {}) : Parent_(std::move(parent)) {}
    virtual void BeforeCall() {}
    const std::shared_ptr<THandle>& GetParent() const { return Parent_; }
    bool HasChildren() const;
    std::vector<std::shared_ptr<THandle>> GetChildren() const;
    bool IsRetired() const { return Retired_.load(std::memory_order_relaxed); }
    void Retire() { Retired_.store(true, std::memory_order_relaxed); }
    std::unique_lock<std::mutex> LockOperation() const { return std::unique_lock(OperationMutex_); }

protected:
    // API calls hold the connection lifecycle gate before this lock. It protects
    // this handle's mutable state and diagnostics; SDK objects need no extra lock.
    mutable std::mutex OperationMutex_;

private:
    const std::shared_ptr<THandle> Parent_;
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
    std::conditional_t<Exclusive, std::unique_lock<std::shared_mutex>,
        std::shared_lock<std::shared_mutex>> connection;
    if constexpr (!std::is_same_v<Handle, TEnvironment>) {
        if constexpr (Exclusive) {
            connection = owner->GetConnection().LockExclusive();
        } else {
            connection = owner->GetConnection().LockShared();
        }
    }
    auto lock = owner->LockOperation();
    if (owner->IsRetired()) {
        return SQL_INVALID_HANDLE;
    }
    if constexpr (Mode != ECallMode::Diagnostic) {
        owner->ClearErrors();
    }
    try {
        if constexpr (Mode == ECallMode::Ordinary) {
            owner->BeforeCall();
        }
        const SQLRETURN ret = InvokeOdbc(std::forward<Fn>(func), owner.get());
        if constexpr (Mode == ECallMode::Ordinary) {
            owner->SetLastReturnCode(ret);
        }
        return ret;
    } catch (...) {
        if constexpr (Mode == ECallMode::Diagnostic) {
            return SQL_ERROR;
        }
        return RecordCurrentException(*owner);
    }
}

} // namespace NYdb::NOdbc
