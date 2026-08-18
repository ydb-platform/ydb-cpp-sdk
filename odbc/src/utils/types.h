#pragma once

#include <ydb-cpp-sdk/client/value/value.h>

#include <sql.h>
#include <sqlext.h>
#include <optional>

namespace NYdb {
namespace NOdbc {

struct TYdbTypeInfo {
    SQLSMALLINT SqlType = SQL_UNKNOWN_TYPE;
    SQLULEN ColumnSize = 4096;
    SQLSMALLINT Nullable = SQL_NO_NULLS;
    std::optional<SQLSMALLINT> DecimalDigits;
    std::optional<SQLSMALLINT> Radix;
    bool Unsigned = false;
};

TYdbTypeInfo DescribeYdbType(const TType& type);
SQLULEN GetColumnSize(SQLSMALLINT sqlType);

} // namespace NOdbc
} // namespace NYdb
