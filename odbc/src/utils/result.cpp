#include "result.h"

#include "convert.h"

namespace NYdb {
namespace NOdbc {

class TCommonResultSet : public IResultSet {
public:
    SQLRETURN BindCol(SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                      SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) override {
        BoundColumns_.erase(std::remove_if(BoundColumns_.begin(), BoundColumns_.end(),
            [columnNumber](const TBoundColumn& col) { return col.ColumnNumber == columnNumber; }), BoundColumns_.end());
        if (!targetValue) {
            return SQL_SUCCESS;
        }
        BoundColumns_.push_back({columnNumber, targetType, targetValue, bufferLength, strLenOrInd});
        return SQL_SUCCESS;
    }

protected:
    void FillBoundColumns() {
        for (const auto& col : BoundColumns_) {
            GetData(col.ColumnNumber, col.TargetType, col.TargetValue, col.BufferLength, col.StrLenOrInd);
        }
    }

protected:
    std::vector<TBoundColumn> BoundColumns_;
};

class TExecResultSet : public TCommonResultSet {
public:
    TExecResultSet(NYdb::NQuery::TExecuteQueryIterator iterator)
        : Iterator_(std::move(iterator)) {}

    bool Fetch() override {
        while (true) {
            if (ResultSetParser_) {
                if (ResultSetParser_->TryNextRow()) {
                    FillBoundColumns();
                    return true;
                }
                ResultSetParser_.reset();
            }
            auto part = Iterator_.ReadNext().ExtractValueSync();
            if (part.EOS()) {
                return false;
            }
            if (!part.IsSuccess()) {
                return false;
            }
            if (part.HasResultSet()) {
                ResultSetParser_ = std::make_unique<TResultSetParser>(part.ExtractResultSet());
            }
        }
        return false;
    }

    SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                      SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) override {
        if (!ResultSetParser_) {
            return SQL_NO_DATA;
        }
        if (columnNumber < 1 || columnNumber > ResultSetParser_->ColumnsCount()) {
            return SQL_ERROR;
        }
        return ConvertColumn(ResultSetParser_->ColumnParser(columnNumber - 1), targetType, targetValue, bufferLength, strLenOrInd);
    }

    size_t ColumnsCount() const override {
        return ResultSetParser_ ? ResultSetParser_->ColumnsCount() : 0;
    }

    const TColumnMeta& GetColumnMeta(size_t index) const override {
        // TODO: implement return column metadata
        static TColumnMeta dummy;
        return dummy;
    }

private:
    NYdb::NQuery::TExecuteQueryIterator Iterator_;
    std::unique_ptr<TResultSetParser> ResultSetParser_;
};

class TVirtualResultSet : public TCommonResultSet {
public:
    TVirtualResultSet(const std::vector<TColumnMeta>& columns, const TTable& table)
        : Columns_(columns), Table_(table) {
            std::cout << "TVirtualResultSet constructor" << std::endl;
            std::cout << "Columns count: " << Columns_.size() << std::endl;
            std::cout << "Table size: " << Table_.size() << std::endl;
        }

    bool Fetch() override {
        std::cout << "Fetching row " << Cursor_ << std::endl;
        Cursor_++;
        if (Cursor_ >= static_cast<std::int64_t>(Table_.size())) {
            return false;
        }
        FillBoundColumns();
        return true;
    }

    SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                      SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) override {
        if (Cursor_ >= static_cast<std::int64_t>(Table_.size())) {
            return SQL_NO_DATA;
        }
        if (Cursor_ < 0 || columnNumber < 1 || columnNumber > Columns_.size()) {
            return SQL_ERROR;
        }
        TValueParser parser{Table_[Cursor_][columnNumber - 1]};
        return ConvertColumn(parser, targetType, targetValue, bufferLength, strLenOrInd);
    }

    size_t ColumnsCount() const override {
        return Columns_.size();
    }

    const TColumnMeta& GetColumnMeta(size_t index) const override {
        return Columns_[index];
    }

private:
    std::vector<TColumnMeta> Columns_;
    TTable Table_;
    int64_t Cursor_ = -1;
};

std::unique_ptr<IResultSet> CreateExecResultSet(NYdb::NQuery::TExecuteQueryIterator iterator) {
    return std::make_unique<TExecResultSet>(std::move(iterator));
}

std::unique_ptr<IResultSet> CreateVirtualResultSet(const std::vector<TColumnMeta>& columns, const TTable& table) {
    return std::make_unique<TVirtualResultSet>(columns, table);
}

} // namespace NOdbc
} // namespace NYdb
