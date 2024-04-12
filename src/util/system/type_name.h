#pragma once

#include <src/util/string/subst.h>

#include <typeindex>
#include <typeinfo>

// Consider using TypeName function family.
std::string CppDemangle(const std::string& name);

// TypeName function family return human readable type name.

std::string TypeName(const std::type_info& typeInfo);
std::string TypeName(const std::type_index& typeInfo);

// Works for types known at compile-time
// (thus, does not take any inheritance into account)
template <class T>
inline std::string TypeName() {
    return TypeName(typeid(T));
}

// Works for dynamic type, including complex class hierarchies.
// Also, distinguishes between T, T*, T const*, T volatile*, T const volatile*,
// but not T and T const.
template <class T>
inline std::string TypeName(const T& t) {
    return TypeName(typeid(t));
}
