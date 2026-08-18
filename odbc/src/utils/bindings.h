#pragma once

#include <sql.h>
#include <sqlext.h>

#include <ydb-cpp-sdk/client/types/fwd.h>

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
};

} // namespace NYdb::NOdbc
