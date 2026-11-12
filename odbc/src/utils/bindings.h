#pragma once

#include "odbc_compat.h"

namespace NYdb::NOdbc {

enum class EWideTemporal {
    None,
    Date32,
    Datetime64,
    Timestamp64,
};

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
    EWideTemporal WideTemporal = EWideTemporal::None;
};

inline bool BoundParamIsNull(const TBoundParam& param) noexcept {
    return param.IsNullData
        || (param.StrLenOrIndPtr && *param.StrLenOrIndPtr == SQL_NULL_DATA);
}

} // namespace NYdb::NOdbc
