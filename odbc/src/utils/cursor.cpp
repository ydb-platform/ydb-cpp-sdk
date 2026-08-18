#include "cursor.h"
#include "types.h"

#include <ydb-cpp-sdk/client/result/result.h>

namespace NYdb::NOdbc {

class TExecCursor : public ICursor {
public:
    explicit TExecCursor(TResultSet resultSet)
        : Parser_(resultSet) {
        for (const auto& col : resultSet.GetColumnsMeta()) {
            const TYdbTypeInfo type = DescribeYdbType(col.Type);
            Columns_.push_back({col.Name, type.SqlType, type.ColumnSize, type.Nullable,
                                type.DecimalDigits.value_or(0), type.Unsigned});
        }
    }

    bool Fetch() override {
        return Parser_.TryNextRow();
    }

    SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                      SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd,
                      SQLLEN* offset) override {
        if (columnNumber < 1 || columnNumber > Parser_.ColumnsCount()) {
            return SQL_ERROR;
        }
        return ConvertColumn(
            Parser_.ColumnParser(columnNumber - 1), targetType, targetValue, bufferLength, strLenOrInd,
            offset);
    }

private:
    TResultSetParser Parser_;
};

class TVirtualCursor : public ICursor {
public:
    TVirtualCursor(TColumnSchema columns, TTable table)
        : Table_(std::move(table)) {
        Columns_.assign(columns.begin(), columns.end());
    }

    bool Fetch() override {
        return ++Cursor_ < static_cast<std::int64_t>(Table_.size());
    }

    SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                      SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd,
                      SQLLEN* offset) override {
        if (Cursor_ >= static_cast<std::int64_t>(Table_.size())) {
            return SQL_NO_DATA;
        }
        if (Cursor_ < 0 || columnNumber < 1 || columnNumber > Columns_.size()) {
            return SQL_ERROR;
        }
        return ConvertColumn(Table_[Cursor_][columnNumber - 1], targetType,
                             targetValue, bufferLength, strLenOrInd, offset);
    }

private:
    TTable Table_;
    int64_t Cursor_ = -1;
};

std::unique_ptr<ICursor> CreateExecCursor(const NQuery::TExecuteQueryResult& result) {
    return result.GetResultSets().empty()
        ? nullptr
        : std::make_unique<TExecCursor>(result.GetResultSet(0));
}

std::unique_ptr<ICursor> CreateVirtualCursor(TColumnSchema columns, TTable table) {
    return std::make_unique<TVirtualCursor>(columns, std::move(table));
}

} // namespace NYdb::NOdbc
