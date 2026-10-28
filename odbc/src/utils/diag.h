#pragma once

#include "error_manager.h"

#include <string>
#include <string_view>
#include <algorithm>
#include <cstring>
#include <limits>

namespace NYdb::NOdbc {
namespace Diag {

inline SQLRETURN AddNullPointer(TErrorManager& errors) {
    return errors.AddError("HY009", 0, "Invalid use of null pointer");
}

inline SQLRETURN AddNotImplemented(TErrorManager& errors) {
    return errors.AddError("HYC00", 0, "Optional feature not implemented");
}

inline SQLRETURN AddInvalidAttrValue(TErrorManager& errors, std::string_view name) {
    return errors.AddError("HY024", 0, "Invalid " + std::string(name) + " value");
}

inline SQLRETURN AddInvalidBufferLength(TErrorManager& errors) {
    return errors.AddError("HY090", 0, "Invalid string or buffer length");
}

inline SQLRETURN AddRightTruncated(TErrorManager& errors) {
    return errors.AddError("01004", 0, "String data, right truncated", SQL_SUCCESS_WITH_INFO);
}

enum class EStringWriteMode : unsigned char {
    Odbc,
    OptionalOdbc,
    Diagnostic,
    ConnectionAttribute,
    Descriptor,
    ColumnAttribute,
};

template<EStringWriteMode Mode, typename TLength>
SQLRETURN WriteString(TErrorManager* errors, std::string_view value, SQLPOINTER output,
                      TLength bufferLength, TLength* length) {
    const SQLLEN fullLength = static_cast<SQLLEN>(value.size());
    const auto reportLength = [&] {
        if (length) {
            if constexpr (Mode == EStringWriteMode::Odbc
                          || Mode == EStringWriteMode::Diagnostic
                          || Mode == EStringWriteMode::ColumnAttribute) {
                *length = static_cast<TLength>(std::min<SQLLEN>(
                    fullLength, std::numeric_limits<TLength>::max()));
            } else {
                *length = static_cast<TLength>(fullLength);
            }
        }
    };
    if constexpr (Mode == EStringWriteMode::OptionalOdbc) {
        if (bufferLength < 0) {
            return AddInvalidBufferLength(*errors);
        }
        reportLength();
        if (!output) {
            return SQL_SUCCESS;
        }
    }
    if constexpr (Mode == EStringWriteMode::Diagnostic
                  || Mode == EStringWriteMode::ConnectionAttribute
                  || Mode == EStringWriteMode::Descriptor) {
        reportLength();
    }
    if constexpr (Mode == EStringWriteMode::ColumnAttribute) {
        if (bufferLength < 0 || (!output && bufferLength != 0)) {
            return AddInvalidBufferLength(*errors);
        }
    }
    if (!output) {
        if constexpr (Mode == EStringWriteMode::Diagnostic
                      || Mode == EStringWriteMode::ConnectionAttribute
                      || Mode == EStringWriteMode::Descriptor) {
            return SQL_SUCCESS;
        } else if constexpr (Mode == EStringWriteMode::Odbc) {
            return AddNullPointer(*errors);
        }
    }
    if (bufferLength < 0) {
        if constexpr (Mode == EStringWriteMode::Diagnostic) {
            return SQL_ERROR;
        }
        return AddInvalidBufferLength(*errors);
    }
    if constexpr (Mode == EStringWriteMode::Odbc
                  || Mode == EStringWriteMode::ColumnAttribute) {
        reportLength();
    }
    if constexpr (Mode == EStringWriteMode::ConnectionAttribute) {
        if (bufferLength == 0) {
            return AddInvalidBufferLength(*errors);
        }
    }
    if (bufferLength == 0) {
        if (!fullLength) {
            return SQL_SUCCESS;
        }
        if constexpr (Mode == EStringWriteMode::Diagnostic) {
            return SQL_SUCCESS_WITH_INFO;
        }
        return AddRightTruncated(*errors);
    }
    const SQLLEN copied = std::min<SQLLEN>(fullLength, bufferLength - 1);
    if (copied) {
        std::memcpy(output, value.data(), static_cast<size_t>(copied));
    }
    static_cast<char*>(output)[copied] = 0;
    if (copied == fullLength) {
        return SQL_SUCCESS;
    }
    if constexpr (Mode == EStringWriteMode::Diagnostic) {
        return SQL_SUCCESS_WITH_INFO;
    }
    return AddRightTruncated(*errors);
}

inline SQLRETURN WriteOdbcString(TErrorManager& errors, std::string_view value, SQLPOINTER output,
                                 SQLSMALLINT bufferLength, SQLSMALLINT* length) {
    return WriteString<EStringWriteMode::Odbc>(&errors, value, output, bufferLength, length);
}

template<typename TLength>
SQLRETURN WriteOptionalOdbcString(TErrorManager& errors, std::string_view value, SQLPOINTER output,
                                  TLength bufferLength, TLength* length) {
    return WriteString<EStringWriteMode::OptionalOdbc>(
        &errors, value, output, bufferLength, length);
}

} // namespace Diag

} // namespace NYdb::NOdbc
