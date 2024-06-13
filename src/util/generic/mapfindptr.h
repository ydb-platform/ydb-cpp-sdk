#pragma once

#include <ydb-cpp-sdk/util/system/compiler.h>

#include <utility>

/** MapFindPtr usage:

if (T* value = MapFindPtr(myMap, someKey) {
    std::cout << *value;
}

*/

template <class Map, class K>
inline auto MapFindPtr(Map& map Y_LIFETIME_BOUND, const K& key) {
    auto i = map.find(key);

    return (i == map.end() ? nullptr : &i->second);
}

template <class Map, class K>
inline auto MapFindPtr(const Map& map Y_LIFETIME_BOUND, const K& key) {
    auto i = map.find(key);

    return (i == map.end() ? nullptr : &i->second);
}

template <class Map, class K, class DefaultValue>
inline auto MapValue(const Map& map Y_LIFETIME_BOUND, const K& key, DefaultValue&& defaultValue) {
    auto found = MapFindPtr(map, key);
    return found ? *found : std::forward<DefaultValue>(defaultValue);
}
