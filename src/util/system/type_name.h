#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/util/system/type_name.h
#include <ydb-cpp-sdk/util/string/subst.h>
========
#include <src/util/string/subst.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/util/system/type_name.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/util/system/type_name.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

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
