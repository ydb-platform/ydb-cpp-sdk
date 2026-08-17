#pragma once

#include <ydb-cpp-sdk/client/value/value.h>

#include <sql.h>
#include <sqlext.h>
#include <optional>

namespace NYdb {
namespace NOdbc {

SQLSMALLINT GetTypeId(const TType& type);
SQLSMALLINT IsNullable(const TType& type);
SQLULEN GetColumnSize(SQLSMALLINT sqlType);
SQLULEN GetColumnSize(const TType& type);
bool IsUnsigned(const TType& type);

std::optional<SQLSMALLINT> GetDecimalDigits(const TType& type);
std::optional<SQLSMALLINT> GetRadix(const TType& type);

} // namespace NOdbc
} // namespace NYdb
