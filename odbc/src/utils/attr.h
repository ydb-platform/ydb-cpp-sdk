#pragma once

#include "diag.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

#include "odbc_compat.h"

namespace NYdb::NOdbc {

inline std::string ReadAttributeString(SQLPOINTER value, SQLINTEGER length) {
    const char* text = static_cast<const char*>(value);
    if (length == SQL_NTS) {
        return std::string(text);
    }
    return length < 0 ? std::string() : std::string(text, static_cast<size_t>(length));
}

template<typename T>
T ReadIntegerAttr(SQLPOINTER value) noexcept {
    if constexpr (std::is_pointer_v<T>) {
        return static_cast<T>(value);
    } else {
        return static_cast<T>(reinterpret_cast<uintptr_t>(value));
    }
}

template<typename TInput, typename TOutput, typename TValid>
SQLRETURN SetCheckedAttribute(
    SQLPOINTER value,
    TOutput& output,
    TErrorManager& errors,
    std::string_view name,
    TValid valid) {
    const TInput input = ReadIntegerAttr<TInput>(value);
    if (!valid(input)) {
        return Diag::AddInvalidAttrValue(errors, name);
    }
    output = static_cast<TOutput>(input);
    return SQL_SUCCESS;
}

template<SQLINTEGER IdValue, auto MemberValue, bool ReadableValue = true, bool WritableValue = true>
struct TScalarProperty {
    static constexpr SQLINTEGER Id = IdValue;
    static constexpr auto Member = MemberValue;
    static constexpr bool Readable = ReadableValue;
    static constexpr bool Writable = WritableValue;
};

template<typename T>
const T* ScalarValue(const T& value) {
    return &value;
}

template<typename T>
const T* ScalarValue(const std::optional<T>& value) {
    return value ? &*value : nullptr;
}

template<typename T>
void SetScalarValue(T& property, SQLPOINTER value) {
    property = ReadIntegerAttr<T>(value);
}

template<typename T>
void SetScalarValue(std::optional<T>& property, SQLPOINTER value) {
    property = ReadIntegerAttr<T>(value);
}

template<typename... TProperties>
class TScalarProperties {
    template<bool Write, typename TObject, typename TCallback>
    static bool Visit(SQLINTEGER id, TObject& object, TCallback&& callback) {
        return (((Write ? TProperties::Writable : TProperties::Readable)
            && id == TProperties::Id
            && (callback(object.*TProperties::Member), true)) || ...);
    }

public:
    template<typename TObject>
    static std::optional<SQLRETURN> Get(
        SQLINTEGER id,
        const TObject& object,
        SQLPOINTER output,
        SQLINTEGER* length = nullptr) {
        SQLRETURN result = SQL_SUCCESS;
        const bool found = Visit<false>(id, object, [&](const auto& property) {
            const auto* scalar = ScalarValue(property);
            if (!scalar) {
                result = SQL_NO_DATA;
                return;
            }
            using TValue = std::remove_cvref_t<decltype(*scalar)>;
            *static_cast<TValue*>(output) = *scalar;
            if (length) {
                *length = sizeof(TValue);
            }
        });
        return found ? std::optional<SQLRETURN>(result) : std::nullopt;
    }

    template<typename TObject>
    static bool Set(SQLINTEGER id, TObject& object, SQLPOINTER value) {
        return Visit<true>(id, object, [&](auto& property) {
            SetScalarValue(property, value);
        });
    }
};

} // namespace NYdb::NOdbc
