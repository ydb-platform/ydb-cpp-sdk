#pragma once

#include "bindings.h"

#include <ydb-cpp-sdk/client/value/value.h>

#include <sql.h>
#include <sqlext.h>

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#define YDB_ODBC_C_INTEGER_TYPES(X)          \
    X(SQL_C_BIT, SQLCHAR)                    \
    X(SQL_C_SBIGINT, SQLBIGINT)              \
    X(SQL_C_UBIGINT, SQLUBIGINT)             \
    X(SQL_C_LONG, SQLINTEGER)                \
    X(SQL_C_SLONG, SQLINTEGER)               \
    X(SQL_C_ULONG, SQLUINTEGER)              \
    X(SQL_C_SHORT, SQLSMALLINT)              \
    X(SQL_C_SSHORT, SQLSMALLINT)             \
    X(SQL_C_USHORT, SQLUSMALLINT)            \
    X(SQL_C_TINYINT, SQLSCHAR)               \
    X(SQL_C_STINYINT, SQLSCHAR)              \
    X(SQL_C_UTINYINT, SQLCHAR)

#define YDB_ODBC_SCALAR_TYPES(X)                         \
    X(Int8, int8_t, SQL_TINYINT, false)                  \
    X(Uint8, uint8_t, SQL_TINYINT, true)                 \
    X(Int16, int16_t, SQL_SMALLINT, false)               \
    X(Uint16, uint16_t, SQL_SMALLINT, true)              \
    X(Int32, int32_t, SQL_INTEGER, false)                \
    X(Uint32, uint32_t, SQL_INTEGER, true)               \
    X(Int64, int64_t, SQL_BIGINT, false)                 \
    X(Uint64, uint64_t, SQL_BIGINT, true)                \
    X(Float, float, SQL_REAL, false)                     \
    X(Double, double, SQL_DOUBLE, false)

namespace NYdb::NOdbc {

template <typename Fn>
auto VisitCInteger(SQLSMALLINT type, Fn&& fn)
    -> std::optional<std::invoke_result_t<Fn, std::type_identity<SQLBIGINT>>> {
#define ODBC_VISIT_C_INTEGER(odbcType, cppType)            \
    case odbcType:                                         \
        return fn(std::type_identity<cppType>{});
    switch (type) {
        YDB_ODBC_C_INTEGER_TYPES(ODBC_VISIT_C_INTEGER)
        default:
            return std::nullopt;
    }
#undef ODBC_VISIT_C_INTEGER
}

struct TSqlTypeSpec {
    SQLSMALLINT Type;
    std::string_view Name;
    std::optional<EPrimitiveType> ParamType;
    std::string_view YqlType;
    SQLULEN ColumnSize;
    bool Advertise;
};

struct TParamTypeSpec {
    std::optional<EPrimitiveType> Type;
    std::string YqlType;
    SQLULEN Precision{};
    SQLSMALLINT Scale{};
};

std::span<const TSqlTypeSpec> GetSqlTypeSpecs();
const TSqlTypeSpec* FindSqlTypeSpec(SQLSMALLINT sqlType);
SQLULEN GetCTypeSize(SQLSMALLINT cType, SQLLEN variableLength);
std::string MapSqlTypeToken(std::string_view sqlType);
std::optional<TParamTypeSpec> ResolveParamType(const TBoundParam& param);

} // namespace NYdb::NOdbc
