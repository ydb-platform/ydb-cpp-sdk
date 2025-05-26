#pragma once

#include <ydb-cpp-sdk/client/params/params.h>

#include <sql.h>
#include <sqlext.h>

namespace NYdb {
namespace NOdbc {

struct TBoundParam {
    SQLUSMALLINT ParamNumber;
    SQLSMALLINT InputOutputType;
    SQLSMALLINT ValueType;
    SQLSMALLINT ParameterType;
    SQLULEN ColumnSize;
    SQLSMALLINT DecimalDigits;
    SQLPOINTER ParameterValuePtr;
    SQLLEN BufferLength;
    SQLLEN* StrLenOrIndPtr;
};

void ConvertValue(const TBoundParam& param, TParamValueBuilder& builder);

} // namespace NYdb
} // namespace NOdbc
