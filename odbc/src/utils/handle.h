#pragma once

#include "error_manager.h"

#include <util/generic/scope.h>

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <utility>
#include <vector>

namespace NYdb::NOdbc {

class TEnvironment;
class TConnection;
class TStatement;

enum class ECallMode : unsigned char { Ordinary, Diagnostic, Consuming };

class THandle : public TErrorManager {
public:
    explicit THandle(THandle* parent = nullptr) : Parent_(parent) {}
    THandle* GetParent() const { return Parent_; }
    // The caller holds this parent's operation lock for child-list access.
    void AddChild(THandle* child);
    void RemoveChild(THandle* child);
    bool HasChildren() const { return !Children_.empty(); }
    const std::vector<THandle*>& GetChildren() const { return Children_; }
    std::vector<THandle*> ReleaseChildren() { return std::exchange(Children_, {}); }
    std::unique_lock<std::mutex> LockOperation() const { return std::unique_lock(OperationMutex_); }

protected:
    // API calls hold the connection lifecycle gate before this lock. It protects
    // this handle's mutable state and diagnostics; SDK objects need no extra lock.
    mutable std::mutex OperationMutex_;

private:
    THandle* const Parent_;
    std::vector<THandle*> Children_;
};

template<class Handle>
Handle* CheckedHandle(SQLHANDLE handle) {
    // Applications must keep handles alive until all uses have completed.
    return dynamic_cast<Handle*>(static_cast<THandle*>(handle));
}

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
    auto* owner = CheckedHandle<Handle>(handlePtr);
    if (!owner) {
        return SQL_INVALID_HANDLE;
    }
    // ENV traversal and DBC removal acquire parent ownership in the same order.
    std::unique_lock<std::mutex> parent;
    if constexpr (Mode == ECallMode::Consuming && std::is_same_v<Handle, TConnection>) {
        parent = owner->GetParent()->LockOperation();
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
    if constexpr (Mode == ECallMode::Consuming
                  && !std::is_same_v<Handle, TEnvironment>
                  && !std::is_same_v<Handle, TConnection>) {
        parent = owner->GetParent()->LockOperation();
    }
    auto lock = owner->LockOperation();
    if constexpr (Mode == ECallMode::Ordinary && std::is_same_v<Handle, TStatement>) {
        owner->StartExecution();
    }
    bool executionFinished = false;
    Y_DEFER {
        if (!executionFinished) {
            if constexpr (Mode == ECallMode::Ordinary && std::is_same_v<Handle, TStatement>) {
                owner->FinishExecution();
            }
        }
    };
    if constexpr (Mode != ECallMode::Diagnostic) {
        owner->ClearErrors();
    }
    try {
        SQLRETURN ret = InvokeOdbc(std::forward<Fn>(func), owner);
        if constexpr (Mode == ECallMode::Ordinary && std::is_same_v<Handle, TStatement>) {
            const bool canceled = owner->FinishExecution();
            executionFinished = true;
            if (canceled && ret == SQL_NEED_DATA) {
                ret = owner->AddError("HY008", 0, "Operation canceled");
            }
        }
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
