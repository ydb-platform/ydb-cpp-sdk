#pragma once
#ifndef ARCADIA_ENUM_INL_H_
#error "Direct inclusion of this file is not allowed, include arcadia_enum.h"
// For the sake of sane code completion.
#include "arcadia_enum.h"
#endif

#include <ydb-cpp-sdk/util/system/type_name.h>

#include <optional>
#include <unordered_map>

namespace NYT::NDetail {

////////////////////////////////////////////////////////////////////////////////

template <class T>
struct TArcadiaEnumTraitsImpl
{
    static constexpr bool IsBitEnum = false;
    static constexpr bool IsStringSerializableEnum = false;

    static std::string_view GetTypeName()
    {
        static const auto Result = TypeName<T>();
        return Result;
    }

    static std::optional<std::string_view> FindLiteralByValue(T value)
    {
        auto names = GetEnumNames<T>();
        auto it = names.find(value);
        return it == names.end() ? std::nullopt : std::make_optional(std::string_view(it->second));
    }

    static std::optional<T> FindValueByLiteral(std::string_view literal)
    {
        static const auto LiteralToValue = [] {
            std::unordered_map<std::string, T> result;
            for (const auto& [value, name] : GetEnumNames<T>()) {
                result.emplace(name, value);
            }
            return result;
        }();
        auto it = LiteralToValue.find(literal);
        return it == LiteralToValue.end() ? std::nullopt : std::make_optional(it->second);
    }
};

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NDetail
