#pragma once

#include "bindings.h"

#include <sql.h>
#include <sqlext.h>

#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace NYdb::NOdbc {

enum class EParamYdbType {
    Bool,
    Int8,
    Uint8,
    Int16,
    Uint16,
    Int32,
    Uint32,
    Int64,
    Uint64,
    Float,
    Double,
    Decimal,
    Utf8,
    String,
    Date,
    Datetime,
    Timestamp,
};

struct TSqlTypeSpec {
    SQLSMALLINT Type;
    std::string_view Name;
    EParamYdbType ParamType;
    std::string_view YqlType;
    SQLULEN ColumnSize;
    bool Advertise;
};

struct TParamTypeSpec {
    EParamYdbType Type;
    std::string YqlType;
    SQLULEN Precision = 0;
    SQLSMALLINT Scale = 0;
};

std::span<const TSqlTypeSpec> GetSqlTypeSpecs();
const TSqlTypeSpec* FindSqlTypeSpec(SQLSMALLINT sqlType);
std::string MapSqlTypeToken(std::string_view sqlType);
std::optional<TParamTypeSpec> ResolveParamType(const TBoundParam& param);
std::string FormatYqlParamDeclareType(const TBoundParam& param);

} // namespace NYdb::NOdbc
