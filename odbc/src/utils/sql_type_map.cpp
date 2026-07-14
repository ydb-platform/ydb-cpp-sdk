#include "sql_type_map.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>

namespace NYdb::NOdbc {
namespace {

constexpr std::array TypeSpecs{
    TSqlTypeSpec{SQL_BIGINT, "BIGINT", "Int64", 19, true},
    TSqlTypeSpec{SQL_INTEGER, "INTEGER", "Int32", 10, true},
    TSqlTypeSpec{SQL_SMALLINT, "SMALLINT", "Int16", 5, true},
    TSqlTypeSpec{SQL_DOUBLE, "DOUBLE", "Double", 15, true},
    TSqlTypeSpec{SQL_REAL, "REAL", "Float", 7, true},
    TSqlTypeSpec{SQL_VARCHAR, "VARCHAR", "Utf8", 255, true},
    TSqlTypeSpec{SQL_CHAR, "CHAR", "Utf8", 255, true},
    TSqlTypeSpec{SQL_LONGVARCHAR, "LONGVARCHAR", "Utf8", 4096, false},
    TSqlTypeSpec{SQL_WCHAR, "WCHAR", "Utf8", 255, false},
    TSqlTypeSpec{SQL_WVARCHAR, "WVARCHAR", "Utf8", 255, false},
    TSqlTypeSpec{SQL_WLONGVARCHAR, "WLONGVARCHAR", "Utf8", 4096, false},
    TSqlTypeSpec{SQL_BIT, "BIT", "Bool", 1, false},
    TSqlTypeSpec{SQL_TINYINT, "TINYINT", "Int8", 3, false},
    TSqlTypeSpec{SQL_FLOAT, "FLOAT", "Double", 15, false},
    TSqlTypeSpec{SQL_DECIMAL, "DECIMAL", "Decimal(22, 9)", 22, false},
    TSqlTypeSpec{SQL_NUMERIC, "NUMERIC", "Decimal(22, 9)", 22, false},
    TSqlTypeSpec{SQL_BINARY, "BINARY", "String", 4096, false},
    TSqlTypeSpec{SQL_VARBINARY, "VARBINARY", "String", 4096, false},
    TSqlTypeSpec{SQL_LONGVARBINARY, "LONGVARBINARY", "String", 4096, false},
    TSqlTypeSpec{SQL_TYPE_DATE, "DATE", "Date", 10, false},
    TSqlTypeSpec{SQL_TYPE_TIME, "TIME", "Time", 8, false},
    TSqlTypeSpec{SQL_TYPE_TIMESTAMP, "TIMESTAMP", "Datetime", 26, false},
};

std::string ToUpperAscii(std::string_view value) {
    std::string upper(value);
    std::ranges::transform(upper, upper.begin(), [](unsigned char byte) {
        return static_cast<char>(std::toupper(byte));
    });
    return upper;
}

} // namespace

std::span<const TSqlTypeSpec> GetSqlTypeSpecs() {
    return TypeSpecs;
}

const TSqlTypeSpec* FindSqlTypeSpec(SQLSMALLINT sqlType) {
    const auto it = std::ranges::find(TypeSpecs, sqlType, &TSqlTypeSpec::Type);
    return it == TypeSpecs.end() ? nullptr : &*it;
}

std::string MapSqlTypeToken(std::string_view sqlType) {
    std::string key = ToUpperAscii(sqlType);
    if (key.starts_with("SQL_")) key.erase(0, 4);
    if (key.starts_with("TYPE_")) key.erase(0, 5);
    const auto it = std::ranges::find(TypeSpecs, key, &TSqlTypeSpec::Name);
    return it == TypeSpecs.end() ? key : std::string(it->YqlType);
}

std::string FormatYqlParamDeclareType(SQLSMALLINT sqlType) {
    const TSqlTypeSpec* spec = FindSqlTypeSpec(sqlType);
    return (spec ? std::string(spec->YqlType) : std::to_string(sqlType)) + '?';
}

} // namespace NYdb::NOdbc
