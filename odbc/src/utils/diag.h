#pragma once

#include "error_manager.h"

#include <string>
#include <string_view>
#include <algorithm>
#include <cstring>

namespace NYdb::NOdbc {
namespace Diag {
    
    inline SQLRETURN AddNullPointer(TErrorManager& errors) {
        return errors.AddError("HY009", 0, "Invalid use of null pointer");
    }
    
    inline SQLRETURN AddNotImplemented(TErrorManager& errors) {
        return errors.AddError("HYC00", 0, "Optional feature not implemented");
    }
    
    inline SQLRETURN AddInvalidAttrValue(TErrorManager& errors, std::string_view attrName) {
        return errors.AddError("HY024", 0, "Invalid " + std::string(attrName) + " value");
    }
    
    inline SQLRETURN AddInvalidBufferLength(TErrorManager& errors) {
        return errors.AddError("HY090", 0, "Invalid string or buffer length");
    }
    
    inline SQLRETURN AddRightTruncated(TErrorManager& errors) {
        return errors.AddError("01004", 0, "String data, right truncated", SQL_SUCCESS_WITH_INFO);
    }

    inline SQLRETURN WriteOdbcString(
        TErrorManager& errors,
        std::string_view value,
        SQLPOINTER outPtr,
        SQLSMALLINT bufferLength,
        SQLSMALLINT* lengthPtr) {
        if (!outPtr) {
            return errors.AddError("HY009", 0, "Invalid use of null pointer");
        }
        if (bufferLength < 0) {
            return errors.AddError("HY090", 0, "Invalid string or buffer length");
        }
        const SQLLEN fullLen = static_cast<SQLLEN>(value.size());
        const SQLSMALLINT reportedLen = static_cast<SQLSMALLINT>(std::min<SQLLEN>(fullLen, 32767));
        if (lengthPtr) {
            *lengthPtr = reportedLen;
        }
        if (bufferLength == 0) {
            return fullLen == 0 ? SQL_SUCCESS : AddRightTruncated(errors);
        }
        auto* out = reinterpret_cast<char*>(outPtr);
        const SQLSMALLINT copyLen = static_cast<SQLSMALLINT>(std::min<SQLLEN>(fullLen, static_cast<SQLLEN>(bufferLength - 1)));
        if (copyLen > 0) {
            std::memcpy(out, value.data(), static_cast<size_t>(copyLen));
        }
        out[copyLen] = '\0';
        if (copyLen < fullLen) {
            return AddRightTruncated(errors);
        }
        return SQL_SUCCESS;
    }

} // namespace Diag

} // namespace NYdb::NOdbc
