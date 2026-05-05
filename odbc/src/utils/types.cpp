#include "types.h"

namespace NYdb {
namespace NOdbc {

SQLSMALLINT GetTypeId(const TType& type) {
    TTypeParser typeParser(type);
    size_t openedOptionals = 0;
    while (typeParser.GetKind() == TTypeParser::ETypeKind::Optional) {
        typeParser.OpenOptional();
        ++openedOptionals;
    }

    auto closeOpenedOptionals = [&]() {
        while (openedOptionals > 0) {
            typeParser.CloseOptional();
            --openedOptionals;
        }
    };

    const auto kind = typeParser.GetKind();
    if (kind == TTypeParser::ETypeKind::Primitive) {
        const auto primitive = typeParser.GetPrimitive();
        closeOpenedOptionals();
        switch (primitive) {
            case EPrimitiveType::Bool:
                return SQL_BIT;
            case EPrimitiveType::Int8:
            case EPrimitiveType::Uint8:
                return SQL_TINYINT;
            case EPrimitiveType::Int16:
            case EPrimitiveType::Uint16:
                return SQL_SMALLINT;
            case EPrimitiveType::Int32:
            case EPrimitiveType::Uint32:
                return SQL_INTEGER;
            case EPrimitiveType::Int64:
            case EPrimitiveType::Uint64:
                return SQL_BIGINT;
            case EPrimitiveType::Float:
                return SQL_REAL;
            case EPrimitiveType::Double:
                return SQL_DOUBLE;
            case EPrimitiveType::Date:
            case EPrimitiveType::Date32:
            case EPrimitiveType::TzDate:
                return SQL_TYPE_DATE;
            case EPrimitiveType::Datetime:
            case EPrimitiveType::Timestamp:
            case EPrimitiveType::Datetime64:
            case EPrimitiveType::Timestamp64:
            case EPrimitiveType::TzDatetime:
            case EPrimitiveType::TzTimestamp:
                return SQL_TYPE_TIMESTAMP;
            case EPrimitiveType::Interval:
            case EPrimitiveType::Interval64:
                return SQL_BIGINT;
            case EPrimitiveType::String:
                return SQL_VARBINARY;
            case EPrimitiveType::Utf8:
            case EPrimitiveType::Yson:
            case EPrimitiveType::Json:
            case EPrimitiveType::JsonDocument:
            case EPrimitiveType::DyNumber:
                return SQL_VARCHAR;
            case EPrimitiveType::Uuid:
                return SQL_GUID;
        }
    }

    closeOpenedOptionals();
    if (kind == TTypeParser::ETypeKind::Decimal) {
        return SQL_DECIMAL;
    }
    return SQL_UNKNOWN_TYPE;
}

SQLSMALLINT IsNullable(const TType& type) {
    TTypeParser typeParser(type);
    if (typeParser.GetKind() == TTypeParser::ETypeKind::Optional || typeParser.GetKind() == TTypeParser::ETypeKind::Null) {
        return SQL_NULLABLE;
    }

    return SQL_NO_NULLS;
}

std::optional<SQLSMALLINT> GetDecimalDigits(const TType& type) {
    TTypeParser typeParser(type);
    if (typeParser.GetKind() != TTypeParser::ETypeKind::Primitive) {
        return std::nullopt;
    }

    switch (typeParser.GetPrimitive()) {
        case EPrimitiveType::Int64:
        case EPrimitiveType::Uint64:
        case EPrimitiveType::Int32:
        case EPrimitiveType::Uint32:
        case EPrimitiveType::Int16:
        case EPrimitiveType::Uint16:
        case EPrimitiveType::Int8:
        case EPrimitiveType::Uint8:
            return 0;
        default:
            return std::nullopt;
    }
}

std::optional<SQLSMALLINT> GetRadix(const TType& type) {
    TTypeParser typeParser(type);
    if (typeParser.GetKind() != TTypeParser::ETypeKind::Primitive) {
        return std::nullopt;
    }

    switch (typeParser.GetPrimitive()) {
        case EPrimitiveType::Int64:
        case EPrimitiveType::Uint64:
        case EPrimitiveType::Int32:
        case EPrimitiveType::Uint32:
        case EPrimitiveType::Int16:
        case EPrimitiveType::Uint16:
        case EPrimitiveType::Int8:
        case EPrimitiveType::Uint8:
            return 10;
        default:
            return std::nullopt;
    }
}

} // namespace NOdbc
} // namespace NYdb
