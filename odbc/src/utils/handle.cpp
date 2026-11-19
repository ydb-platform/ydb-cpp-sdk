#include "handle.h"

#include <algorithm>

namespace NYdb::NOdbc {

void THandle::AddChild(THandle* child) {
    Children_.push_back(child);
}

void THandle::RemoveChild(THandle* child) {
    std::erase(Children_, child);
}

} // namespace NYdb::NOdbc
