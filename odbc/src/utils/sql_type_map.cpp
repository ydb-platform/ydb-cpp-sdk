#include "sql_type_map.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <utility>

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
    TSqlTypeSpec{SQL_TYPE_TIME, "TIME", "Datetime", 8, false},
    TSqlTypeSpec{SQL_TYPE_TIMESTAMP, "TIMESTAMP", "Timestamp", 26, false},
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

    if (param.ParameterType == SQL_TYPE_DATE || param.ParameterType == SQL_DATE) {
        return simple(EParamYdbType::Date, "Date");
    }
    if (param.ParameterType == SQL_TYPE_TIME || param.ParameterType == SQL_TIME) {
        return simple(EParamYdbType::Datetime, "Datetime");
    }
    if (param.ParameterType == SQL_TYPE_TIMESTAMP || param.ParameterType == SQL_TIMESTAMP) {
        return simple(EParamYdbType::Timestamp, "Timestamp");
    }

    switch (param.ParameterType) {
        case SQL_BIGINT:
            if (auto type = unsignedInteger()) return type;
            return simple(EParamYdbType::Int64, "Int64");
        case SQL_INTEGER:
            if (auto type = unsignedInteger()) return type;
            return simple(EParamYdbType::Int32, "Int32");
        case SQL_SMALLINT:
            if (auto type = unsignedInteger()) return type;
            return simple(EParamYdbType::Int16, "Int16");
        case SQL_TINYINT:
            if (auto type = unsignedInteger()) return type;
            return simple(EParamYdbType::Int8, "Int8");
        case SQL_BIT:
            return simple(EParamYdbType::Bool, "Bool");
        case SQL_REAL:
            return simple(EParamYdbType::Float, "Float");
        case SQL_FLOAT:
        case SQL_DOUBLE:
            return simple(EParamYdbType::Double, "Double");
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
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
            return simple(EParamYdbType::Utf8, "Utf8");
        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
            return simple(EParamYdbType::String, "String");
        default:
            return std::nullopt;
    }
}

std::string FormatYqlParamDeclareType(const TBoundParam& param) {
    const auto type = ResolveParamType(param);
    return type ? type->YqlType + '?' : std::to_string(param.ParameterType) + '?';
}

} // namespace NYdb::NOdbc
