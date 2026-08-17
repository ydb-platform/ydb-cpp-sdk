#include "cursor.h"
#include "convert.h"
#include "types.h"

#include <ydb-cpp-sdk/client/result/result.h>

namespace NYdb {
namespace NOdbc {

class TExecCursor : public ICursor {
public:
    explicit TExecCursor(TResultSet resultSet)
        : Parser_(resultSet) {
        for (const auto& col : resultSet.GetColumnsMeta()) {
            const SQLSMALLINT sqlType = GetTypeId(col.Type);
            Columns_.push_back({col.Name, sqlType, GetColumnSize(col.Type), IsNullable(col.Type),
                                GetDecimalDigits(col.Type).value_or(0), IsUnsigned(col.Type)});
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

    const std::vector<TColumnMeta>& GetColumnMeta() const override {
        return Columns_;
    }

private:
    TResultSetParser Parser_;
    std::vector<TColumnMeta> Columns_;
};

class TVirtualCursor : public ICursor {
public:
    TVirtualCursor(const std::vector<TColumnMeta>& columns, const TTable& table)
        : Columns_(columns)
        , Table_(table)
    {}

    bool Fetch() override {
        Cursor_++;
        if (Cursor_ >= static_cast<std::int64_t>(Table_.size())) {
            return false;
        }
        return true;
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
        TValueParser parser{Table_[Cursor_][columnNumber - 1]};
        return ConvertColumn(parser, targetType, targetValue, bufferLength, strLenOrInd, offset);
    }

    const std::vector<TColumnMeta>& GetColumnMeta() const override {
        return Columns_;
    }

private:
    std::vector<TColumnMeta> Columns_;
    TTable Table_;
    int64_t Cursor_ = -1;
};

std::unique_ptr<ICursor> CreateExecCursor(const NQuery::TExecuteQueryResult& result) {
    return result.GetResultSets().empty()
        ? nullptr
        : std::make_unique<TExecCursor>(result.GetResultSet(0));
}

std::unique_ptr<ICursor> CreateVirtualCursor(const std::vector<TColumnMeta>& columns, const TTable& table) {
    return std::make_unique<TVirtualCursor>(columns, table);
}

} // namespace NOdbc
} // namespace NYdb
