#pragma once

#include <sql.h>
#include <sqlext.h>

#include <string>
#include <string_view>

namespace NYdb::NOdbc {

std::string MapSqlTypeToken(std::string_view sqlType);
std::string FormatYqlParamDeclareType(SQLSMALLINT sqlType);

} // namespace NYdb::NOdbc
