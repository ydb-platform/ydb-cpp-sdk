#pragma once

#include <sql.h>
#include <sqlext.h>

#include <span>
#include <string>
#include <string_view>

namespace NYdb::NOdbc {

struct TSqlTypeSpec {
    SQLSMALLINT Type;
    std::string_view Name;
    std::string_view YqlType;
    SQLULEN ColumnSize;
    bool Advertise;
};

std::span<const TSqlTypeSpec> GetSqlTypeSpecs();
const TSqlTypeSpec* FindSqlTypeSpec(SQLSMALLINT sqlType);
std::string MapSqlTypeToken(std::string_view sqlType);
std::string FormatYqlParamDeclareType(SQLSMALLINT sqlType);

} // namespace NYdb::NOdbc
