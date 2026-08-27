#pragma once

#include <ydb-cpp-sdk/client/value/value.h>

#include "odbc_compat.h"
#include <optional>
#include <string>

namespace NYdb::NOdbc {

struct TYdbTypeInfo {
    std::string TypeName;
    SQLSMALLINT SqlType = SQL_UNKNOWN_TYPE;
    SQLULEN ColumnSize = 4096;
    SQLSMALLINT Nullable = SQL_NO_NULLS;
    std::optional<SQLSMALLINT> DecimalDigits;
    std::optional<SQLSMALLINT> Radix;
    bool Unsigned = false;
};

TYdbTypeInfo DescribeYdbType(const TType& type);

} // namespace NYdb::NOdbc
