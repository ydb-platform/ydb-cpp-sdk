#include "sql_type_map.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <unordered_map>

namespace NYdb::NOdbc {

namespace {

std::string ToUpperAscii(std::string_view sv) {
    std::string upper;
    upper.resize(sv.size());
    std::transform(sv.begin(), sv.end(), upper.begin(), [](unsigned char byte) {
        return static_cast<char>(std::toupper(byte));
    });
    return upper;
}

const std::unordered_map<std::string, std::string>& SqlTypeTokenToYqlMap() {
    static const std::unordered_map<std::string, std::string> kMap = {
        {"CHAR", "Utf8"},
        {"VARCHAR", "Utf8"},
        {"LONGVARCHAR", "Utf8"},
        {"WCHAR", "Utf8"},
        {"WVARCHAR", "Utf8"},
        {"WLONGVARCHAR", "Utf8"},
        {"BIT", "Bool"},
        {"TINYINT", "Int8"},
        {"SMALLINT", "Int16"},
        {"INTEGER", "Int32"},
        {"BIGINT", "Int64"},
        {"REAL", "Float"},
        {"FLOAT", "Double"},
        {"DOUBLE", "Double"},
        {"DECIMAL", "Decimal(22, 9)"},
        {"NUMERIC", "Decimal(22, 9)"},
        {"BINARY", "String"},
        {"VARBINARY", "String"},
        {"LONGVARBINARY", "String"},
        {"DATE", "Date"},
        {"TIME", "Time"},
        {"TIMESTAMP", "Datetime"},
        {"TYPE_DATE", "Date"},
        {"TYPE_TIME", "Time"},
        {"TYPE_TIMESTAMP", "Datetime"},
    };
    return kMap;
}

const std::unordered_map<SQLSMALLINT, std::string_view>& OdbcSqlTypeTokens() {
    static const std::unordered_map<SQLSMALLINT, std::string_view> kMap = {
        {SQL_CHAR, "CHAR"},
        {SQL_VARCHAR, "VARCHAR"},
        {SQL_LONGVARCHAR, "LONGVARCHAR"},
        {SQL_WCHAR, "WCHAR"},
        {SQL_WVARCHAR, "WVARCHAR"},
        {SQL_WLONGVARCHAR, "WLONGVARCHAR"},
        {SQL_BIT, "BIT"},
        {SQL_TINYINT, "TINYINT"},
        {SQL_SMALLINT, "SMALLINT"},
        {SQL_INTEGER, "INTEGER"},
        {SQL_BIGINT, "BIGINT"},
        {SQL_REAL, "REAL"},
        {SQL_FLOAT, "FLOAT"},
        {SQL_DOUBLE, "DOUBLE"},
        {SQL_DECIMAL, "DECIMAL"},
        {SQL_NUMERIC, "NUMERIC"},
        {SQL_BINARY, "BINARY"},
        {SQL_VARBINARY, "VARBINARY"},
        {SQL_LONGVARBINARY, "LONGVARBINARY"},
        {SQL_TYPE_DATE, "TYPE_DATE"},
        {SQL_TYPE_TIME, "TYPE_TIME"},
        {SQL_TYPE_TIMESTAMP, "TYPE_TIMESTAMP"},
    };
    return kMap;
}

} // namespace

std::string MapSqlTypeToken(std::string_view sqlType) {
    std::string key = ToUpperAscii(sqlType);
    const std::string kSql = "SQL_";
    if (key.size() > kSql.size() && key.compare(0, kSql.size(), kSql) == 0) {
        key.erase(0, kSql.size());
    }
    const auto& map = SqlTypeTokenToYqlMap();
    const auto mapped = map.find(key);
    if (mapped != map.end()) {
        return mapped->second;
    }
    return key;
}

std::string FormatYqlParamDeclareType(SQLSMALLINT sqlType) {
    const auto& tokens = OdbcSqlTypeTokens();
    const auto tokenIt = tokens.find(sqlType);
    const std::string yql = tokenIt != tokens.end()
        ? MapSqlTypeToken(tokenIt->second)
        : MapSqlTypeToken(std::to_string(sqlType));
    return yql + '?';
}

} // namespace NYdb::NOdbc
