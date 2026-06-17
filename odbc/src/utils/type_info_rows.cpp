#include "type_info_rows.h"

#include <ydb-cpp-sdk/client/value/value.h>

#include <algorithm>
#include <vector>

namespace NYdb::NOdbc {
namespace {

struct TTypeInfoSpec {
    const char* TypeName;
    SQLSMALLINT DataType;
    SQLULEN ColumnSize;
    SQLSMALLINT Nullable;
};

const TTypeInfoSpec kTypeInfoSpecs[] = {
    {"bigint", SQL_BIGINT, 19, SQL_NULLABLE},
    {"integer", SQL_INTEGER, 10, SQL_NULLABLE},
    {"smallint", SQL_SMALLINT, 5, SQL_NULLABLE},
    {"double", SQL_DOUBLE, 53, SQL_NULLABLE},
    {"real", SQL_REAL, 24, SQL_NULLABLE},
    {"varchar", SQL_VARCHAR, 255, SQL_NULLABLE},
    {"char", SQL_CHAR, 255, SQL_NULLABLE},
};

TValue MakeOptionalInt16(SQLSMALLINT value) {
    return TValueBuilder().OptionalInt16(value).Build();
}

TValue MakeOptionalInt32(SQLINTEGER value) {
    return TValueBuilder().OptionalInt32(value).Build();
}

TValue MakeNullUtf8() {
    return TValueBuilder().OptionalUtf8(std::nullopt).Build();
}

std::vector<TValue> MakeTypeInfoRow(const TTypeInfoSpec& spec) {
    return {
        TValueBuilder().Utf8(spec.TypeName).Build(),
        TValueBuilder().Int16(spec.DataType).Build(),
        MakeOptionalInt32(static_cast<SQLINTEGER>(spec.ColumnSize)),
        MakeNullUtf8(),
        MakeNullUtf8(),
        MakeNullUtf8(),
        MakeOptionalInt16(spec.Nullable),
        MakeOptionalInt16(SQL_FALSE),
        MakeOptionalInt16(SQL_PRED_SEARCHABLE),
        MakeNullUtf8(),
        MakeOptionalInt16(SQL_FALSE),
        MakeOptionalInt16(SQL_FALSE),
        TValueBuilder().OptionalUtf8(spec.TypeName).Build(),
        MakeOptionalInt16(0),
        MakeOptionalInt16(0),
        MakeOptionalInt16(spec.DataType),
        MakeOptionalInt16(0),
        MakeOptionalInt32(10),
        MakeOptionalInt32(0),
    };
}

} // namespace

TTable BuildTypeInfoRows(SQLSMALLINT dataType) {
    TTable table;
    for (const auto& spec : kTypeInfoSpecs) {
        if (dataType != SQL_ALL_TYPES && spec.DataType != dataType) {
            continue;
        }
        table.push_back(MakeTypeInfoRow(spec));
    }
    return table;
}

} // namespace NYdb::NOdbc
