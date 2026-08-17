#include "type_info_rows.h"
#include "sql_type_map.h"

#include <ydb-cpp-sdk/client/value/value.h>

#include <algorithm>
#include <cctype>
#include <vector>

namespace NYdb::NOdbc {
namespace {

TValue MakeOptionalInt16(SQLSMALLINT value) {
    return TValueBuilder().OptionalInt16(value).Build();
}

TValue MakeOptionalInt32(SQLINTEGER value) {
    return TValueBuilder().OptionalInt32(value).Build();
}

TValue MakeNullUtf8() {
    return TValueBuilder().OptionalUtf8(std::nullopt).Build();
}

std::vector<TValue> MakeTypeInfoRow(const TSqlTypeSpec& spec) {
    std::string typeName(spec.Name);
    std::ranges::transform(typeName, typeName.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return {
        TValueBuilder().Utf8(typeName).Build(),
        TValueBuilder().Int16(spec.Type).Build(),
        MakeOptionalInt32(static_cast<SQLINTEGER>(spec.ColumnSize)),
        MakeNullUtf8(),
        MakeNullUtf8(),
        MakeNullUtf8(),
        MakeOptionalInt16(SQL_NULLABLE),
        MakeOptionalInt16(SQL_FALSE),
        MakeOptionalInt16(SQL_PRED_SEARCHABLE),
        MakeNullUtf8(),
        MakeOptionalInt16(SQL_FALSE),
        MakeOptionalInt16(SQL_FALSE),
        TValueBuilder().OptionalUtf8(typeName).Build(),
        MakeOptionalInt16(0),
        MakeOptionalInt16(0),
        MakeOptionalInt16(spec.Type),
        MakeOptionalInt16(0),
        MakeOptionalInt32(10),
        MakeOptionalInt32(0),
    };
}

} // namespace

TTable BuildTypeInfoRows(SQLSMALLINT dataType) {
    if (dataType == SQL_DATE) {
        dataType = SQL_TYPE_DATE;
    } else if (dataType == SQL_TIME) {
        dataType = SQL_TYPE_TIME;
    } else if (dataType == SQL_TIMESTAMP) {
        dataType = SQL_TYPE_TIMESTAMP;
    }
    TTable table;
    for (const TSqlTypeSpec& spec : GetSqlTypeSpecs()) {
        if ((dataType == SQL_ALL_TYPES && !spec.Advertise)
            || (dataType != SQL_ALL_TYPES && spec.Type != dataType)) {
            continue;
        }
        table.push_back(MakeTypeInfoRow(spec));
    }
    return table;
}

} // namespace NYdb::NOdbc
