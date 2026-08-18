#include "types.h"
#include "sql_type_map.h"

#include <array>
#include <type_traits>

namespace NYdb::NOdbc {
namespace {

struct TPrimitiveSpec {
    EPrimitiveType Type;
    SQLSMALLINT SqlType;
    bool Integer = false;
    bool Unsigned = false;
};

constexpr std::array PrimitiveSpecs{
    TPrimitiveSpec{EPrimitiveType::Bool, SQL_BIT},
#define ODBC_PRIMITIVE_SPEC(name, cppType, sqlType, isUnsigned) \
    TPrimitiveSpec{EPrimitiveType::name, sqlType, std::is_integral_v<cppType>, isUnsigned},
    YDB_ODBC_SCALAR_TYPES(ODBC_PRIMITIVE_SPEC)
#undef ODBC_PRIMITIVE_SPEC
    TPrimitiveSpec{EPrimitiveType::Interval, SQL_BIGINT},
    TPrimitiveSpec{EPrimitiveType::Interval64, SQL_BIGINT},
    TPrimitiveSpec{EPrimitiveType::Date, SQL_TYPE_DATE},
    TPrimitiveSpec{EPrimitiveType::Date32, SQL_TYPE_DATE},
    TPrimitiveSpec{EPrimitiveType::TzDate, SQL_TYPE_DATE},
    TPrimitiveSpec{EPrimitiveType::Datetime, SQL_TYPE_TIME},
    TPrimitiveSpec{EPrimitiveType::Timestamp, SQL_TYPE_TIMESTAMP},
    TPrimitiveSpec{EPrimitiveType::Datetime64, SQL_TYPE_TIMESTAMP},
    TPrimitiveSpec{EPrimitiveType::Timestamp64, SQL_TYPE_TIMESTAMP},
    TPrimitiveSpec{EPrimitiveType::TzDatetime, SQL_TYPE_TIMESTAMP},
    TPrimitiveSpec{EPrimitiveType::TzTimestamp, SQL_TYPE_TIMESTAMP},
    TPrimitiveSpec{EPrimitiveType::String, SQL_VARBINARY},
    TPrimitiveSpec{EPrimitiveType::Utf8, SQL_VARCHAR},
    TPrimitiveSpec{EPrimitiveType::Yson, SQL_VARCHAR},
    TPrimitiveSpec{EPrimitiveType::Json, SQL_VARCHAR},
    TPrimitiveSpec{EPrimitiveType::JsonDocument, SQL_VARCHAR},
    TPrimitiveSpec{EPrimitiveType::DyNumber, SQL_VARCHAR},
    TPrimitiveSpec{EPrimitiveType::Uuid, SQL_GUID},
};

SQLULEN GetColumnSize(SQLSMALLINT sqlType) {
    const TSqlTypeSpec* spec = FindSqlTypeSpec(sqlType);
    return spec ? spec->ColumnSize : sqlType == SQL_GUID ? 36 : 4096;
}

} // namespace

TYdbTypeInfo DescribeYdbType(const TType& type) {
    TYdbTypeInfo info;
    TTypeParser parser(type);
    info.Nullable = parser.GetKind() == TTypeParser::ETypeKind::Optional
            || parser.GetKind() == TTypeParser::ETypeKind::Null
        ? SQL_NULLABLE
        : SQL_NO_NULLS;
    size_t optionals = 0;
    while (parser.GetKind() == TTypeParser::ETypeKind::Optional) {
        parser.OpenOptional();
        ++optionals;
    }
    if (parser.GetKind() == TTypeParser::ETypeKind::Decimal) {
        const TDecimalType decimal = parser.GetDecimal();
        info.SqlType = SQL_DECIMAL;
        info.ColumnSize = decimal.Precision;
        info.DecimalDigits = decimal.Scale;
        info.Radix = 10;
    } else if (parser.GetKind() == TTypeParser::ETypeKind::Primitive) {
        const EPrimitiveType type = parser.GetPrimitive();
        const auto it = std::ranges::find(PrimitiveSpecs, type, &TPrimitiveSpec::Type);
        if (it != PrimitiveSpecs.end()) {
            info.SqlType = it->SqlType;
            info.ColumnSize = GetColumnSize(info.SqlType);
            if (it->Integer) {
                info.Unsigned = it->Unsigned;
                info.DecimalDigits = 0;
                info.Radix = 10;
            }
        }
    }
    while (optionals--) {
        parser.CloseOptional();
    }
    return info;
}

} // namespace NYdb::NOdbc
