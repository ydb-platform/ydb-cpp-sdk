#include <ydb-cpp-sdk/c_api/result.h>

#include <ydb-cpp-sdk/client/proto/accessor.h>
#include <ydb-cpp-sdk/client/result/result.h>

#include "impl/result_impl.h" // NOLINT
#include "impl/value_impl.h" // NOLINT

extern "C" {

int YdbGetColumnsCount(TYdbResultSet* resultSet) {
    if (!resultSet || !resultSet->result.has_value()) {
        return -1;
    }

    return resultSet->result->ColumnsCount();
}

int YdbGetRowsCount(TYdbResultSet* resultSet) {
    if (!resultSet || !resultSet->result.has_value()) {
        return -1;
    }

    return resultSet->result->RowsCount();
}

int YdbIsTruncated(TYdbResultSet* resultSet) {
    if (!resultSet || !resultSet->result.has_value()) {
        return -1;
    }

    return resultSet->result->Truncated();
}

const char* YdbGetColumnName(TYdbResultSet* resultSet, size_t index) {
    if (!resultSet || !resultSet->result.has_value()) {
        return nullptr;
    }

    return resultSet->result->GetColumnsMeta()[index].Name.c_str();
}

int YdbGetColumnIndex(TYdbResultSet* resultSet, const char* name) {
    try {
        if (!resultSet || !resultSet->result.has_value()) {
            return -1;
        }

        NYdb::TResultSetParser parser(*resultSet->result);
        return parser.ColumnIndex(name);
    } catch (...) {
        return -1;
    }
}

TYdbValue* YdbGetValue(TYdbResultSet* resultSet, size_t rowIndex, const char* name) {
    try {
        if (!resultSet || !resultSet->result.has_value()) {
            return new TYdbValue{YDB_VALUE_ERROR, "Invalid result set"};
        }

        NYdb::TResultSetParser parser(*resultSet->result);
        int columnIndex = parser.ColumnIndex(name);
        if (columnIndex == -1) {
            return new TYdbValue{YDB_VALUE_ERROR, "Invalid column name"};
        }

        return YdbGetValueByIndex(resultSet, rowIndex, columnIndex);
    } catch (...) {
        return nullptr;
    }
}

TYdbValue* YdbGetValueByIndex(TYdbResultSet* resultSet, size_t rowIndex, size_t columnIndex) {
    try {
        if (!resultSet || !resultSet->result.has_value()) {
            return nullptr;
        }

        auto proto = NYdb::TProtoAccessor::GetProto(*resultSet->result);

        auto type = resultSet->result->GetColumnsMeta()[columnIndex].Type;
        auto value = proto.rows(rowIndex).items(columnIndex);

        return new TYdbValue{YDB_VALUE_OK, "", NYdb::TValue{type, value}};
    } catch (...) {
        return nullptr;
    }
}

void YdbDestroyResultSet(TYdbResultSet* resultSet) {
    delete resultSet;
}

}
