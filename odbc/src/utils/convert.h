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

struct TBoundColumn {
    SQLUSMALLINT ColumnNumber;
    SQLSMALLINT TargetType;
    SQLPOINTER TargetValue;
    SQLLEN BufferLength;
    SQLLEN* StrLenOrInd;
};

SQLRETURN ConvertParam(const TBoundParam& param, TParamValueBuilder& builder);
SQLRETURN ConvertColumn(TValueParser& parser, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd);

} // namespace NYdb
} // namespace NOdbc
