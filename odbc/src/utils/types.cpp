#include "types.h"

namespace NYdb {
namespace NOdbc {

SQLSMALLINT GetTypeId(const TType& type) {
    // TODO: implement
    return 0;
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
            return 64;
        case EPrimitiveType::Uint64:
            return 64;
        case EPrimitiveType::Int32:
            return 32;
        case EPrimitiveType::Uint32:
            return 32;
        case EPrimitiveType::Int16:
            return 16;
        case EPrimitiveType::Uint16:
            return 16;
        case EPrimitiveType::Int8:
            return 8;
        case EPrimitiveType::Uint8:
            return 8;
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
