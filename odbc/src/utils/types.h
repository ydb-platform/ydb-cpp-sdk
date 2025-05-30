#pragma once

#include <ydb-cpp-sdk/client/value/value.h>

#include <sql.h>

namespace NYdb {
namespace NOdbc {

SQLINTEGER GetTypeId(const TType& type);
std::optional<SQLSMALLINT> GetDecimalDigits(const TType& type);
std::optional<SQLSMALLINT> GetRadix(const TType& type);

} // namespace NOdbc
} // namespace NYdb
