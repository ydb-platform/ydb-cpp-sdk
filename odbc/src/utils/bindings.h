#pragma once

#include "odbc_compat.h"

namespace NYdb::NOdbc {

struct TBoundParam {
    SQLUSMALLINT ParamNumber;
    SQLSMALLINT ValueType;
    SQLSMALLINT ParameterType;
    SQLULEN ColumnSize;
    SQLSMALLINT DecimalDigits;
    SQLPOINTER ParameterValuePtr;
    SQLLEN BufferLength;
    SQLLEN* StrLenOrIndPtr;
    bool AtExec = false;
    bool IsNullData = false;
};

inline bool BoundParamIsNull(const TBoundParam& param) noexcept {
    return param.IsNullData
        || (param.StrLenOrIndPtr && *param.StrLenOrIndPtr == SQL_NULL_DATA);
}

} // namespace NYdb::NOdbc
