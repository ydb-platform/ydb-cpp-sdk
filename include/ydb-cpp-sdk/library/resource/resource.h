#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace NResource {
    struct TResource {
        std::string_view Key;
        std::string Data;
    };

    typedef std::vector<TResource> TResources;

    bool Has(const std::string_view key);
    std::string Find(const std::string_view key);
    bool FindExact(const std::string_view key, std::string* out);
    /// @note Perform full scan for now.
    void FindMatch(const std::string_view subkey, TResources* out);
    size_t Count() noexcept;
    std::string_view KeyByIndex(size_t idx);
    std::vector<std::string_view> ListAllKeys();
}
