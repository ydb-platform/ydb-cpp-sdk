#include "handle.h"

#include <mutex>
#include <unordered_map>

namespace NYdb::NOdbc {
namespace {
    class THandleRegistry {
    public:
        void Register(SQLHANDLE handle, std::shared_ptr<THandle> owner) {
            std::lock_guard lock(Mutex_);
            Handles_.emplace(handle, std::move(owner));
        }

        std::shared_ptr<THandle> Pin(SQLHANDLE handle) {
            std::lock_guard lock(Mutex_);
            const auto it = Handles_.find(handle);
            return it == Handles_.end() ? nullptr : it->second;
        }

        void Unregister(SQLHANDLE handle) {
            // Destruction can unregister a child. Never run it under the registry lock.
            decltype(Handles_)::node_type removed;
            {
                std::lock_guard lock(Mutex_);
                removed = Handles_.extract(handle);
            }
        }

    private:
        std::mutex Mutex_;
        std::unordered_map<SQLHANDLE, std::shared_ptr<THandle>> Handles_;
    } Registry;
}

void RegisterHandle(SQLHANDLE handle, std::shared_ptr<THandle> owner) { Registry.Register(handle, std::move(owner)); }
std::shared_ptr<THandle> PinHandle(SQLHANDLE handle) { return Registry.Pin(handle); }
void UnregisterHandle(SQLHANDLE handle) { Registry.Unregister(handle); }

} // namespace NYdb::NOdbc
