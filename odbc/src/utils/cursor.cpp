#include "cursor.h"

#include "convert.h"
#include "types.h"

#include <ydb-cpp-sdk/client/result/result.h>

namespace NYdb {
namespace NOdbc {

class TExecCursor : public ICursor {
public:
    TExecCursor(IBindingFiller* bindingFiller, NQuery::TExecuteQueryIterator iterator,
            std::optional<NQuery::TExecuteQueryPart> prefetchedPart)
        : BindingFiller_(bindingFiller)
        , Iterator_(std::move(iterator))
        , PrefetchedPart_(std::move(prefetchedPart))
    {}

    bool Fetch() override {
        while (true) {
            if (ResultSetParser_) {
                if (ResultSetParser_->TryNextRow()) {
                    BindingFiller_->FillBoundColumns();
                    return true;
                }
                ResultSetParser_.reset();
            }
            NQuery::TExecuteQueryPart part = [&]() {
                if (PrefetchedPart_) {
                    auto p = std::move(*PrefetchedPart_);
                    PrefetchedPart_.reset();
                    return p;
                }
                return Iterator_.ReadNext().ExtractValueSync();
            }();
            if (part.EOS()) {
                return false;
            }
            if (!part.IsSuccess()) {
                BindingFiller_->OnStreamPartError(part);
                return false;
            }
            if (part.HasResultSet()) {
                TResultSet rs = part.ExtractResultSet();
                Columns_.clear();
                Columns_.reserve(rs.ColumnsCount());
                for (const auto& col : rs.GetColumnsMeta()) {
                    Columns_.push_back(TColumnMeta{
                        col.Name,
                        GetTypeId(col.Type),
                        0,
                        IsNullable(col.Type)});
                }
                ResultSetParser_ = std::make_unique<TResultSetParser>(rs);
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

    const std::vector<TColumnMeta>& GetColumnMeta() const override {
        return Columns_;
    }

private:
    // void GetNextPart() {
    //     auto part = Iterator_.ReadNext().ExtractValueSync();
    //     while (!part.EOS() && part.IsSuccess() && !part.HasResultSet()) {
    //         part = Iterator_.ReadNext().ExtractValueSync();
    //     }
    //     Part_ = std::move(part);
    // }

    IBindingFiller* BindingFiller_;
    NQuery::TExecuteQueryIterator Iterator_;
    std::optional<NQuery::TExecuteQueryPart> PrefetchedPart_;
    std::unique_ptr<TResultSetParser> ResultSetParser_;
    std::vector<TColumnMeta> Columns_;
};

class TVirtualCursor : public ICursor {
public:
    TVirtualCursor(IBindingFiller* bindingFiller, const std::vector<TColumnMeta>& columns, const TTable& table)
        : BindingFiller_(bindingFiller)
        , Columns_(columns)
        , Table_(table)
    {}

    bool Fetch() override {
        Cursor_++;
        if (Cursor_ >= static_cast<std::int64_t>(Table_.size())) {
            return false;
        }
        BindingFiller_->FillBoundColumns();
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

    const std::vector<TColumnMeta>& GetColumnMeta() const override {
        return Columns_;
    }

private:
    IBindingFiller* BindingFiller_;
    std::vector<TColumnMeta> Columns_;
    TTable Table_;
    int64_t Cursor_ = -1;
};

std::unique_ptr<ICursor> CreateExecCursor(IBindingFiller* bindingFiller,
        NQuery::TExecuteQueryIterator iterator,
        std::optional<NQuery::TExecuteQueryPart> prefetchedPart) {
    return std::make_unique<TExecCursor>(bindingFiller, std::move(iterator), std::move(prefetchedPart));
}

std::unique_ptr<ICursor> CreateVirtualCursor(IBindingFiller* bindingFiller, const std::vector<TColumnMeta>& columns, const TTable& table) {
    return std::make_unique<TVirtualCursor>(bindingFiller, columns, table);
}

} // namespace NOdbc
} // namespace NYdb
