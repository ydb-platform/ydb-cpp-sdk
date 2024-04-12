#pragma once

#include <string_view>

// can be used for caseless hashes like: THashMap<std::string_view, T, TCIOps, TCIOps>

struct TCIOps {
    size_t operator()(const char* s) const noexcept;
    size_t operator()(const std::string_view& s) const noexcept;

    bool operator()(const char* f, const char* s) const noexcept;
    bool operator()(const std::string_view& f, const std::string_view& s) const noexcept;
};
