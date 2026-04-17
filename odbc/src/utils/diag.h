#pragma once

#include "error_manager.h"

#include <string>
#include <string_view>

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

}

} // namespace NYdb::NOdbc::Diag
