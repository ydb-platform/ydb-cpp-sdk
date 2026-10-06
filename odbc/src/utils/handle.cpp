#include "handle.h"
#include "error_manager.h"

#include <mutex>
#include <unordered_map>

namespace NYdb::NOdbc {
namespace {
    std::mutex Mutex;
    std::unordered_map<SQLHANDLE, std::shared_ptr<TErrorManager>> Handles;
}

void RegisterHandle(SQLHANDLE handle, std::shared_ptr<TErrorManager> owner) {
    std::lock_guard lock(Mutex);
    Handles.emplace(handle, std::move(owner));
}

std::shared_ptr<TErrorManager> PinHandle(SQLHANDLE handle) {
    std::lock_guard lock(Mutex);
    const auto it = Handles.find(handle);
    return it == Handles.end() ? nullptr : it->second;
}

void UnregisterHandle(SQLHANDLE handle) {
    // Destruction can unregister a child. Never run it under the registry lock.
    decltype(Handles)::node_type removed;
    {
        std::lock_guard lock(Mutex);
        removed = Handles.extract(handle);
    }
}

} // namespace NYdb::NOdbc
