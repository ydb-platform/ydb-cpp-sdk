#pragma once

#include "error_manager.h"

#include <initializer_list>
#include <optional>
#include <string>

#include <sql.h>
#include <sqlext.h>

namespace NYdb::NOdbc {

std::string ReadAttributeString(SQLPOINTER value, SQLINTEGER stringLength);

SQLRETURN WriteAttributeString(
    const std::string& source,
    SQLPOINTER value,
    SQLINTEGER bufferLength,
    SQLINTEGER* stringLengthPtr,
    TErrorManager& errors);

template<typename T>
T ReadIntegerAttr(SQLPOINTER value) noexcept {
    return static_cast<T>(reinterpret_cast<uintptr_t>(value));
}

template<typename T>
std::optional<T> ReadIntegerAttrIfIn(SQLPOINTER value, std::initializer_list<T> allowed) noexcept {
    const T token = ReadIntegerAttr<T>(value);
    for (const T allowedValue : allowed) {
        if (token == allowedValue) {
            return token;
        }
    }
    return std::nullopt;
}

} // namespace NYdb::NOdbc
