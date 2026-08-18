#include "types.h"
#include "sql_type_map.h"

namespace NYdb {
namespace NOdbc {

SQLULEN GetColumnSize(SQLSMALLINT sqlType) {
    const TSqlTypeSpec* spec = FindSqlTypeSpec(sqlType);
    return spec ? spec->ColumnSize : sqlType == SQL_GUID ? 36 : 4096;
}

TYdbTypeInfo DescribeYdbType(const TType& type) {
    TYdbTypeInfo info;
    TTypeParser parser(type);
    info.Nullable = parser.GetKind() == TTypeParser::ETypeKind::Optional
        || parser.GetKind() == TTypeParser::ETypeKind::Null
        ? SQL_NULLABLE
        : SQL_NO_NULLS;

    size_t openedOptionals = 0;
    while (parser.GetKind() == TTypeParser::ETypeKind::Optional) {
        parser.OpenOptional();
        ++openedOptionals;
    }

    auto closeOpenedOptionals = [&]() {
        while (openedOptionals > 0) {
            parser.CloseOptional();
            --openedOptionals;
        }
    };

    const auto kind = parser.GetKind();
    if (kind == TTypeParser::ETypeKind::Decimal) {
        const TDecimalType decimal = parser.GetDecimal();
        info.SqlType = SQL_DECIMAL;
        info.ColumnSize = decimal.Precision;
        info.DecimalDigits = decimal.Scale;
        info.Radix = 10;
        closeOpenedOptionals();
        return info;
    }
    if (kind != TTypeParser::ETypeKind::Primitive) {
        closeOpenedOptionals();
        return info;
    }

    const EPrimitiveType primitive = parser.GetPrimitive();
    closeOpenedOptionals();
    switch (primitive) {
        case EPrimitiveType::Bool:
            info.SqlType = SQL_BIT;
            break;
        case EPrimitiveType::Int8:
        case EPrimitiveType::Uint8:
            info.SqlType = SQL_TINYINT;
            break;
        case EPrimitiveType::Int16:
        case EPrimitiveType::Uint16:
            info.SqlType = SQL_SMALLINT;
            break;
        case EPrimitiveType::Int32:
        case EPrimitiveType::Uint32:
            info.SqlType = SQL_INTEGER;
            break;
        case EPrimitiveType::Int64:
        case EPrimitiveType::Uint64:
            info.SqlType = SQL_BIGINT;
            break;
        case EPrimitiveType::Float:
            info.SqlType = SQL_REAL;
            break;
        case EPrimitiveType::Double:
            info.SqlType = SQL_DOUBLE;
            break;
        case EPrimitiveType::Date:
        case EPrimitiveType::Date32:
        case EPrimitiveType::TzDate:
            info.SqlType = SQL_TYPE_DATE;
            break;
        case EPrimitiveType::Datetime:
            info.SqlType = SQL_TYPE_TIME;
            break;
        case EPrimitiveType::Timestamp:
        case EPrimitiveType::Datetime64:
        case EPrimitiveType::Timestamp64:
        case EPrimitiveType::TzDatetime:
        case EPrimitiveType::TzTimestamp:
            info.SqlType = SQL_TYPE_TIMESTAMP;
            break;
        case EPrimitiveType::Interval:
        case EPrimitiveType::Interval64:
            info.SqlType = SQL_BIGINT;
            break;
        case EPrimitiveType::String:
            info.SqlType = SQL_VARBINARY;
            break;
        case EPrimitiveType::Utf8:
        case EPrimitiveType::Yson:
        case EPrimitiveType::Json:
        case EPrimitiveType::JsonDocument:
        case EPrimitiveType::DyNumber:
            info.SqlType = SQL_VARCHAR;
            break;
        case EPrimitiveType::Uuid:
            info.SqlType = SQL_GUID;
            break;
    }

    info.ColumnSize = GetColumnSize(info.SqlType);
    switch (primitive) {
        case EPrimitiveType::Uint8:
        case EPrimitiveType::Uint16:
        case EPrimitiveType::Uint32:
        case EPrimitiveType::Uint64:
            info.Unsigned = true;
            [[fallthrough]];
        case EPrimitiveType::Int8:
        case EPrimitiveType::Int16:
        case EPrimitiveType::Int32:
        case EPrimitiveType::Int64:
            info.DecimalDigits = 0;
            info.Radix = 10;
            break;
        default:
            break;
    }
    return info;
}

} // namespace NOdbc
} // namespace NYdb
