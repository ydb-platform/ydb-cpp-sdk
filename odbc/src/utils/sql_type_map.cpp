#include "sql_type_map.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <utility>

namespace NYdb::NOdbc {
namespace {

constexpr std::array TypeSpecs{
    TSqlTypeSpec{SQL_BIGINT, "BIGINT", EParamYdbType::Int64, "Int64", 19, true},
    TSqlTypeSpec{SQL_INTEGER, "INTEGER", EParamYdbType::Int32, "Int32", 10, true},
    TSqlTypeSpec{SQL_SMALLINT, "SMALLINT", EParamYdbType::Int16, "Int16", 5, true},
    TSqlTypeSpec{SQL_DOUBLE, "DOUBLE", EParamYdbType::Double, "Double", 15, true},
    TSqlTypeSpec{SQL_REAL, "REAL", EParamYdbType::Float, "Float", 7, true},
    TSqlTypeSpec{SQL_VARCHAR, "VARCHAR", EParamYdbType::Utf8, "Utf8", 255, true},
    TSqlTypeSpec{SQL_CHAR, "CHAR", EParamYdbType::Utf8, "Utf8", 255, true},
    TSqlTypeSpec{SQL_LONGVARCHAR, "LONGVARCHAR", EParamYdbType::Utf8, "Utf8", 4096, false},
    TSqlTypeSpec{SQL_WCHAR, "WCHAR", EParamYdbType::Utf8, "Utf8", 255, false},
    TSqlTypeSpec{SQL_WVARCHAR, "WVARCHAR", EParamYdbType::Utf8, "Utf8", 255, false},
    TSqlTypeSpec{SQL_WLONGVARCHAR, "WLONGVARCHAR", EParamYdbType::Utf8, "Utf8", 4096, false},
    TSqlTypeSpec{SQL_BIT, "BIT", EParamYdbType::Bool, "Bool", 1, false},
    TSqlTypeSpec{SQL_TINYINT, "TINYINT", EParamYdbType::Int8, "Int8", 3, false},
    TSqlTypeSpec{SQL_FLOAT, "FLOAT", EParamYdbType::Double, "Double", 15, false},
    TSqlTypeSpec{SQL_DECIMAL, "DECIMAL", EParamYdbType::Decimal, "Decimal(22, 9)", 22, false},
    TSqlTypeSpec{SQL_NUMERIC, "NUMERIC", EParamYdbType::Decimal, "Decimal(22, 9)", 22, false},
    TSqlTypeSpec{SQL_BINARY, "BINARY", EParamYdbType::String, "String", 4096, false},
    TSqlTypeSpec{SQL_VARBINARY, "VARBINARY", EParamYdbType::String, "String", 4096, false},
    TSqlTypeSpec{SQL_LONGVARBINARY, "LONGVARBINARY", EParamYdbType::String, "String", 4096, false},
    TSqlTypeSpec{SQL_TYPE_DATE, "DATE", EParamYdbType::Date, "Date", 10, false},
    TSqlTypeSpec{SQL_TYPE_TIME, "TIME", EParamYdbType::Datetime, "Datetime", 8, false},
    TSqlTypeSpec{SQL_TYPE_TIMESTAMP, "TIMESTAMP", EParamYdbType::Timestamp, "Timestamp", 26, false},
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

std::optional<TParamTypeSpec> ResolveParamType(const TBoundParam& param) {
    const auto simple = [](EParamYdbType type, std::string yql) {
        return std::optional<TParamTypeSpec>(TParamTypeSpec{type, std::move(yql)});
    };
    const auto unsignedInteger = [&]() -> std::optional<TParamTypeSpec> {
        switch (param.ValueType) {
            case SQL_C_UTINYINT: return simple(EParamYdbType::Uint8, "Uint8");
            case SQL_C_USHORT: return simple(EParamYdbType::Uint16, "Uint16");
            case SQL_C_ULONG: return simple(EParamYdbType::Uint32, "Uint32");
            case SQL_C_UBIGINT: return simple(EParamYdbType::Uint64, "Uint64");
            default: return std::nullopt;
        }
    };

    SQLSMALLINT sqlType = param.ParameterType;
    if (sqlType == SQL_DATE) sqlType = SQL_TYPE_DATE;
    if (sqlType == SQL_TIME) sqlType = SQL_TYPE_TIME;
    if (sqlType == SQL_TIMESTAMP) sqlType = SQL_TYPE_TIMESTAMP;
    const TSqlTypeSpec* spec = FindSqlTypeSpec(sqlType);
    if (!spec) {
        return std::nullopt;
    }

    switch (sqlType) {
        case SQL_BIGINT:
        case SQL_INTEGER:
        case SQL_SMALLINT:
        case SQL_TINYINT:
            if (auto type = unsignedInteger()) return type;
            break;
        case SQL_DECIMAL:
        case SQL_NUMERIC: {
            if (param.DecimalDigits == 0) {
                if (auto type = unsignedInteger()) return type;
            }
            const SQLULEN precision = param.ColumnSize ? param.ColumnSize : 22;
            const SQLSMALLINT scale = param.ColumnSize ? param.DecimalDigits : 9;
            if (precision == 0 || precision > 35 || scale < 0
                || static_cast<SQLULEN>(scale) > precision) {
                return std::nullopt;
            }
            return TParamTypeSpec{
                EParamYdbType::Decimal,
                "Decimal(" + std::to_string(precision) + ", " + std::to_string(scale) + ")",
                precision,
                scale};
        }
        default:
            break;
    }
    return simple(spec->ParamType, std::string(spec->YqlType));
}

std::string FormatYqlParamDeclareType(const TBoundParam& param) {
    const auto type = ResolveParamType(param);
    return type ? type->YqlType + '?' : std::to_string(param.ParameterType) + '?';
}

} // namespace NYdb::NOdbc
