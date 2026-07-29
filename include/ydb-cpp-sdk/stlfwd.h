#pragma once

// STL "forwarding" for the poor

#include <cstddef>
#include <string>
#include <string_view>
#include <version>
#include <utility>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <filesystem>
#include <functional>

#ifdef __cpp_lib_format
namespace std {
    template <class T, class CharT>
    struct formatter;
}
#endif
