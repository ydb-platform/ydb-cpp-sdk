#include "sql_type_map.h"

#include <ydb-cpp-sdk/client/value/value.h>

#include <algorithm>
#include <array>
#include <cctype>

namespace NYdb::NOdbc {
namespace {

constexpr std::array TypeSpecs{
    TSqlTypeSpec{SQL_BIGINT, "BIGINT", EPrimitiveType::Int64, "Int64", 19, true},
    TSqlTypeSpec{SQL_INTEGER, "INTEGER", EPrimitiveType::Int32, "Int32", 10, true},
    TSqlTypeSpec{SQL_SMALLINT, "SMALLINT", EPrimitiveType::Int16, "Int16", 5, true},
    TSqlTypeSpec{SQL_DOUBLE, "DOUBLE", EPrimitiveType::Double, "Double", 15, true},
    TSqlTypeSpec{SQL_REAL, "REAL", EPrimitiveType::Float, "Float", 7, true},
    TSqlTypeSpec{SQL_VARCHAR, "VARCHAR", EPrimitiveType::Utf8, "Utf8", 255, true},
    TSqlTypeSpec{SQL_CHAR, "CHAR", EPrimitiveType::Utf8, "Utf8", 255, true},
    TSqlTypeSpec{SQL_LONGVARCHAR, "LONGVARCHAR", EPrimitiveType::Utf8, "Utf8", 4096, false},
    TSqlTypeSpec{SQL_WCHAR, "WCHAR", EPrimitiveType::Utf8, "Utf8", 255, false},
    TSqlTypeSpec{SQL_WVARCHAR, "WVARCHAR", EPrimitiveType::Utf8, "Utf8", 255, false},
    TSqlTypeSpec{SQL_WLONGVARCHAR, "WLONGVARCHAR", EPrimitiveType::Utf8, "Utf8", 4096, false},
    TSqlTypeSpec{SQL_BIT, "BIT", EPrimitiveType::Bool, "Bool", 1, false},
    TSqlTypeSpec{SQL_TINYINT, "TINYINT", EPrimitiveType::Int8, "Int8", 3, false},
    TSqlTypeSpec{SQL_FLOAT, "FLOAT", EPrimitiveType::Double, "Double", 15, false},
    TSqlTypeSpec{SQL_DECIMAL, "DECIMAL", std::nullopt, "Decimal(22, 9)", 22, false},
    TSqlTypeSpec{SQL_NUMERIC, "NUMERIC", std::nullopt, "Decimal(22, 9)", 22, false},
    TSqlTypeSpec{SQL_BINARY, "BINARY", EPrimitiveType::String, "String", 4096, false},
    TSqlTypeSpec{SQL_VARBINARY, "VARBINARY", EPrimitiveType::String, "String", 4096, false},
    TSqlTypeSpec{SQL_LONGVARBINARY, "LONGVARBINARY", EPrimitiveType::String, "String", 4096, false},
    TSqlTypeSpec{SQL_TYPE_DATE, "DATE", EPrimitiveType::Date, "Date", 10, false},
    TSqlTypeSpec{SQL_TYPE_TIME, "TIME", EPrimitiveType::Datetime, "Datetime", 8, false},
    TSqlTypeSpec{SQL_TYPE_TIMESTAMP, "TIMESTAMP", EPrimitiveType::Timestamp, "Timestamp", 26, false},
};

std::optional<TParamTypeSpec> UnsignedType(SQLSMALLINT type) {
    switch (type) {
        case SQL_C_UTINYINT: return TParamTypeSpec{EPrimitiveType::Uint8, "Uint8"};
        case SQL_C_USHORT: return TParamTypeSpec{EPrimitiveType::Uint16, "Uint16"};
        case SQL_C_ULONG: return TParamTypeSpec{EPrimitiveType::Uint32, "Uint32"};
        case SQL_C_UBIGINT: return TParamTypeSpec{EPrimitiveType::Uint64, "Uint64"};
        default: return std::nullopt;
    }
}

SQLSMALLINT CanonicalType(SQLSMALLINT type) {
    if (type == SQL_DATE) {
        return SQL_TYPE_DATE;
    }
    if (type == SQL_TIME) {
        return SQL_TYPE_TIME;
    }
    return type == SQL_TIMESTAMP ? SQL_TYPE_TIMESTAMP : type;
}

bool IsIntegerType(SQLSMALLINT type) {
    return type == SQL_BIGINT || type == SQL_INTEGER || type == SQL_SMALLINT
        || type == SQL_TINYINT;
}

} // namespace

std::span<const TSqlTypeSpec> GetSqlTypeSpecs() {
    return TypeSpecs;
}

const TSqlTypeSpec* FindSqlTypeSpec(SQLSMALLINT sqlType) {
    const auto it = std::ranges::find(TypeSpecs, sqlType, &TSqlTypeSpec::Type);
    return it == TypeSpecs.end() ? nullptr : &*it;
}

SQLULEN GetCTypeSize(SQLSMALLINT cType, SQLLEN variableLength) {
    if (cType == SQL_C_CHAR || cType == SQL_C_WCHAR || cType == SQL_C_BINARY) {
        return static_cast<SQLULEN>(std::max<SQLLEN>(variableLength, 0));
    }
    if (const auto size = VisitCInteger(cType, [](auto type) {
            return sizeof(typename decltype(type)::type);
        })) {
        return *size;
    }
    switch (cType) {
        case SQL_C_FLOAT: return sizeof(SQLREAL);
        case SQL_C_DOUBLE: return sizeof(SQLDOUBLE);
        case SQL_C_TYPE_DATE: return sizeof(SQL_DATE_STRUCT);
#if defined(SQL_C_DATE) && SQL_C_DATE != SQL_C_TYPE_DATE
        case SQL_C_DATE: return sizeof(SQL_DATE_STRUCT);
#endif
        case SQL_C_TYPE_TIME: return sizeof(SQL_TIME_STRUCT);
#if defined(SQL_C_TIME) && SQL_C_TIME != SQL_C_TYPE_TIME
        case SQL_C_TIME: return sizeof(SQL_TIME_STRUCT);
#endif
        case SQL_C_TYPE_TIMESTAMP: return sizeof(SQL_TIMESTAMP_STRUCT);
#if defined(SQL_C_TIMESTAMP) && SQL_C_TIMESTAMP != SQL_C_TYPE_TIMESTAMP
        case SQL_C_TIMESTAMP: return sizeof(SQL_TIMESTAMP_STRUCT);
#endif
        case SQL_C_GUID: return sizeof(SQLGUID);
        default: return static_cast<SQLULEN>(std::max<SQLLEN>(variableLength, 0));
    }
}

std::string MapSqlTypeToken(std::string_view sqlType) {
    std::string key(sqlType);
    std::ranges::transform(key, key.begin(), [](unsigned char byte) {
        return static_cast<char>(std::toupper(byte));
    });
    if (key.starts_with("SQL_")) {
        key.erase(0, 4);
    }
    if (key.starts_with("TYPE_")) {
        key.erase(0, 5);
    }
    const auto it = std::ranges::find(TypeSpecs, key, &TSqlTypeSpec::Name);
    return it == TypeSpecs.end() ? key : std::string(it->YqlType);
}

std::optional<TParamTypeSpec> ResolveParamType(const TBoundParam& param) {
    const SQLSMALLINT sqlType = CanonicalType(param.ParameterType);
    const TSqlTypeSpec* spec = FindSqlTypeSpec(sqlType);
    if (!spec) {
        return std::nullopt;
    }
    const bool decimal = sqlType == SQL_DECIMAL || sqlType == SQL_NUMERIC;
    if ((IsIntegerType(sqlType) || (decimal && param.DecimalDigits == 0))) {
        if (auto type = UnsignedType(param.ValueType)) {
            return type;
        }
    }
    if (decimal) {
        const SQLULEN precision = param.ColumnSize ? param.ColumnSize : 22;
        const SQLSMALLINT scale = param.ColumnSize ? param.DecimalDigits : 9;
        if (!precision || precision > 35 || scale < 0 || static_cast<SQLULEN>(scale) > precision) {
            return std::nullopt;
        }
        return TParamTypeSpec{std::nullopt,
                              "Decimal(" + std::to_string(precision) + ", "
                                  + std::to_string(scale) + ")",
                              precision, scale};
    }
    return TParamTypeSpec{spec->ParamType, std::string(spec->YqlType)};
}

} // namespace NYdb::NOdbc
