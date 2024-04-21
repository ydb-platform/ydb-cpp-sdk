#ifndef CAST_INL_H_
#error "Direct inclusion of this file is not allowed, include cast.h"
// For the sake of sane code completion.
#include "cast.h"
#endif

#include <ydb-cpp-sdk/library/yt/misc/enum.h>

#include <src/util/string/builder.h>

#include <src/util/string/cast.h>

#include <concepts>
#include <format>
#include <type_traits>

namespace NYT {

////////////////////////////////////////////////////////////////////////////////

namespace NDetail {

template <class T, class S>
bool IsInIntegralRange(S value)
    requires std::is_signed_v<T> && std::is_signed_v<S>
{
    return value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max();
}

template <class T, class S>
bool IsInIntegralRange(S value)
    requires std::is_signed_v<T> && std::is_unsigned_v<S>
{
    return value <= static_cast<typename std::make_unsigned<T>::type>(std::numeric_limits<T>::max());
}

template <class T, class S>
bool IsInIntegralRange(S value)
    requires std::is_unsigned_v<T> && std::is_signed_v<S>
{
    return value >= 0 && static_cast<typename std::make_unsigned<S>::type>(value) <= std::numeric_limits<T>::max();
}

template <class T, class S>
bool IsInIntegralRange(S value)
    requires std::is_unsigned_v<T> && std::is_unsigned_v<S>
{
    return value <= std::numeric_limits<T>::max();
}

template <class T, class S>
bool IsInIntegralRange(S value)
    requires std::is_enum_v<S>
{
    return IsInIntegralRange<T>(static_cast<std::underlying_type_t<S>>(value));
}

template <class T>
std::string FormatInvalidCastValue(T value)
{
    return ::ToString(value);
}

inline std::string FormatInvalidCastValue(signed char value)
{
    return ::TStringBuilder() << "'" << value << "'";
}

inline std::string FormatInvalidCastValue(unsigned char value)
{
    return ::TStringBuilder() << "'" << value << "'";
}

#ifdef __cpp_char8_t
inline std::string FormatInvalidCastValue(char8_t value)
{
    return FormatInvalidCastValue(static_cast<unsigned char>(value));
}
#endif

} // namespace NDetail

template <class T, class S>
bool TryIntegralCast(S value, T* result)
{
    if (!NYT::NDetail::IsInIntegralRange<T>(value)) {
        return false;
    }
    *result = static_cast<T>(value);
    return true;
}

template <class T, class S>
T CheckedIntegralCast(S value)
{
    T result;
    if (!TryIntegralCast<T>(value, &result)) {
        throw TSimpleException(std::format("Argument value {} is out of expected range",
            NYT::NDetail::FormatInvalidCastValue(value)));
    }
    return result;
}

template <class T, class S>
bool TryEnumCast(S value, T* result)
{
    std::underlying_type_t<T> underlying;
    if (!TryIntegralCast<std::underlying_type_t<T>>(value, &underlying)) {
        return false;
    }
    auto candidate = static_cast<T>(underlying);
    if (!TEnumTraits<T>::FindLiteralByValue(candidate)) {
        return false;
    }
    *result = candidate;
    return true;
}

template <class T, class S>
T CheckedEnumCast(S value)
{
    T result;
    if (!TryEnumCast<T>(value, &result)) {
        throw TSimpleException(std::format("Invalid value {} of enum type {}",
            static_cast<int>(value),
            TEnumTraits<T>::GetTypeName().data()));
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT
