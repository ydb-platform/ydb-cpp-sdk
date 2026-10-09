#include "handle.h"

#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace NYdb::NOdbc {
namespace {
    class THandleRegistry {
    public:
        void Register(SQLHANDLE handle, std::shared_ptr<THandle> owner) {
            const auto* parent = owner->GetParent().get();
            decltype(Handles_)::node_type removed;
            std::lock_guard lock(Mutex_);
            const auto [it, inserted] = Handles_.emplace(handle, std::move(owner));
            if (!inserted) {
                throw std::logic_error("Handle already registered");
            }
            if (parent) {
                try {
                    Children_[parent].insert(handle);
                } catch (...) {
                    const auto children = Children_.find(parent);
                    if (children != Children_.end() && children->second.empty()) {
                        Children_.erase(children);
                    }
                    removed = Handles_.extract(it);
                    throw;
                }
            }
        }

        std::shared_ptr<THandle> Pin(SQLHANDLE handle) {
            std::lock_guard lock(Mutex_);
            const auto it = Handles_.find(handle);
            return it == Handles_.end() ? nullptr : it->second;
        }

        bool HasChildren(const THandle& parent) {
            std::lock_guard lock(Mutex_);
            const auto children = Children_.find(&parent);
            if (children != Children_.end()) {
                for (auto handle : children->second) {
                    if (!Handles_.at(handle)->IsRetired()) {
                        return true;
                    }
                }
            }
            return false;
        }

        std::vector<std::shared_ptr<THandle>> GetChildren(const THandle& parent) {
            std::vector<std::shared_ptr<THandle>> children;
            std::lock_guard lock(Mutex_);
            if (const auto it = Children_.find(&parent); it != Children_.end()) {
                children.reserve(it->second.size());
                for (auto handle : it->second) {
                    const auto& owner = Handles_.at(handle);
                    if (!owner->IsRetired()) {
                        children.push_back(owner);
                    }
                }
            }
            return children;
        }

        void Unregister(SQLHANDLE handle) {
            // Release the registry lock before deferred handle destruction.
            decltype(Handles_)::node_type removed;
            {
                std::lock_guard lock(Mutex_);
                removed = Handles_.extract(handle);
                if (!removed.empty()) {
                    const auto children = Children_.find(removed.mapped()->GetParent().get());
                    if (children != Children_.end()) {
                        children->second.erase(handle);
                        if (children->second.empty()) {
                            Children_.erase(children);
                        }
                    }
                }
            }
        }

    private:
        std::mutex Mutex_;
        std::unordered_map<SQLHANDLE, std::shared_ptr<THandle>> Handles_;
        // Non-owning index maintained atomically with handle publication/removal.
        std::unordered_map<const THandle*, std::unordered_set<SQLHANDLE>> Children_;
    } Registry;
}

void RegisterHandle(SQLHANDLE handle, std::shared_ptr<THandle> owner) { Registry.Register(handle, std::move(owner)); }
std::shared_ptr<THandle> PinHandle(SQLHANDLE handle) { return Registry.Pin(handle); }
void UnregisterHandle(SQLHANDLE handle) { Registry.Unregister(handle); }
bool THandle::HasChildren() const { return Registry.HasChildren(*this); }
std::vector<std::shared_ptr<THandle>> THandle::GetChildren() const { return Registry.GetChildren(*this); }

} // namespace NYdb::NOdbc
