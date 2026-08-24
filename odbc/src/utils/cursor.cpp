#include "cursor.h"
#include "types.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>

namespace NYdb::NOdbc {
namespace {

using TResultRow = std::vector<TValue>;
using TResultRows = std::vector<TResultRow>;

std::vector<TColumnMeta> MakeColumnMeta(const TResultSet& resultSet) {
    std::vector<TColumnMeta> columns;
    columns.reserve(resultSet.GetColumnsMeta().size());
    for (const auto& column : resultSet.GetColumnsMeta()) {
        const TYdbTypeInfo type = DescribeYdbType(column.Type);
        columns.push_back({column.Name, type.SqlType, type.ColumnSize, type.Nullable,
                           type.DecimalDigits.value_or(0), type.Unsigned});
    }
    return columns;
}

TResultRow MaterializeRow(TResultSetParser& parser) {
    TResultRow row;
    row.reserve(parser.ColumnsCount());
    for (size_t column = 0; column < parser.ColumnsCount(); ++column) {
        row.push_back(parser.GetValue(column));
    }
    return row;
}

size_t ToSize(SQLULEN value) {
    const uintmax_t wide = static_cast<uintmax_t>(value);
    return wide > std::numeric_limits<size_t>::max()
        ? std::numeric_limits<size_t>::max()
        : static_cast<size_t>(wide);
}

uintmax_t NegativeMagnitude(SQLLEN value) {
    return static_cast<uintmax_t>(-(value + 1)) + 1;
}

class TForwardCursor final : public ICursor {
public:
    explicit TForwardCursor(const TResultSet& resultSet)
        : ICursor(MakeColumnMeta(resultSet))
        , Parser_(resultSet)
    {}

    TFetchResult Fetch(
        SQLSMALLINT orientation,
        SQLLEN,
        SQLULEN rowsetSize,
        SQLULEN maxRows) override
    {
        CurrentRows_.clear();
        if (orientation != SQL_FETCH_NEXT) {
            return {};
        }

        const size_t wanted = ToSize(rowsetSize);
        const size_t limit = maxRows == 0
            ? Parser_.RowsCount()
            : std::min(Parser_.RowsCount(), ToSize(maxRows));
        CurrentRows_.reserve(wanted);
        while (CurrentRows_.size() < wanted && RowsRead_ < limit
               && Parser_.TryNextRow()) {
            CurrentRows_.push_back(MaterializeRow(Parser_));
            ++RowsRead_;
        }
        return {static_cast<SQLULEN>(CurrentRows_.size()), false};
    }

    SQLULEN GetRowNumber() const override {
        return CurrentRows_.empty()
            ? 0
            : static_cast<SQLULEN>(RowsRead_ - CurrentRows_.size() + 1);
    }

    SQLRETURN GetData(SQLULEN row, SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                      SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd,
                      SQLLEN* offset) override {
        const size_t rowIndex = ToSize(row);
        if (rowIndex >= CurrentRows_.size()) {
            return SQL_NO_DATA;
        }
        if (columnNumber < 1 || columnNumber > Columns_.size()) {
            return SQL_ERROR;
        }
        TValueParser parser(CurrentRows_[rowIndex][columnNumber - 1]);
        return ConvertColumn(
            parser, targetType, targetValue, bufferLength, strLenOrInd, offset);
    }

private:
    TResultSetParser Parser_;
    TResultRows CurrentRows_;
    size_t RowsRead_ = 0;
};

class TStaticCursor final : public ICursor {
public:
    explicit TStaticCursor(const TResultSet& resultSet)
        : ICursor(MakeColumnMeta(resultSet), resultSet.RowsCount())
        , Parser_(resultSet)
    {}

    SQLRETURN GetData(SQLULEN row, SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                      SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd,
                      SQLLEN* offset) override {
        const auto rowIndex = CurrentRow(row);
        if (!rowIndex) {
            return SQL_NO_DATA;
        }
        if (columnNumber < 1 || columnNumber > Columns_.size()) {
            return SQL_ERROR;
        }
        EnsureRow(*rowIndex);
        TValueParser parser(Rows_[*rowIndex][columnNumber - 1]);
        return ConvertColumn(
            parser, targetType, targetValue, bufferLength, strLenOrInd, offset);
    }

private:
    void EnsureRow(size_t rowIndex) {
        while (Rows_.size() <= rowIndex) {
            if (!Parser_.TryNextRow()) {
                throw std::runtime_error("ODBC cursor result ended before its declared row count");
            }
            Rows_.push_back(MaterializeRow(Parser_));
        }
    }

    TResultSetParser Parser_;
    TResultRows Rows_;
};

class TVirtualCursor final : public ICursor {
public:
    TVirtualCursor(TColumnSchema columns, TTable table)
        : ICursor(std::vector<TColumnMeta>(columns.begin(), columns.end()), table.size())
        , Table_(std::move(table))
    {}

    SQLRETURN GetData(SQLULEN row, SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                      SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd,
                      SQLLEN* offset) override {
        const auto rowIndex = CurrentRow(row);
        if (!rowIndex) {
            return SQL_NO_DATA;
        }
        if (columnNumber < 1 || columnNumber > Columns_.size()) {
            return SQL_ERROR;
        }
        return ConvertColumn(Table_[*rowIndex][columnNumber - 1], targetType,
                             targetValue, bufferLength, strLenOrInd, offset);
    }

private:
    TTable Table_;
};

} // namespace

TCursorWindow::TCursorWindow(size_t totalRows)
    : TotalRows_(totalRows)
{}

class TCursorWindow::TPositionResolver {
public:
    struct TTarget {
        EPosition Position;
        size_t Start = 0;
        bool OverlappedStart = false;
    };

    TPositionResolver(
        const TCursorWindow& window,
        size_t rowsetSize,
        SQLULEN requestedRowsetSize,
        size_t visibleRows)
        : Window_(window)
        , RowsetSize_(rowsetSize)
        , RequestedRowsetSize_(static_cast<uintmax_t>(requestedRowsetSize))
        , VisibleRows_(visibleRows)
    {}

    TTarget Resolve(SQLSMALLINT orientation, SQLLEN offset) const {
        switch (orientation) {
            case SQL_FETCH_NEXT: {
                if (Window_.Position_ == EPosition::Before) {
                    return Rowset(0);
                }
                return Window_.Position_ == EPosition::After
                    ? Boundary(EPosition::After)
                    : Positive(static_cast<uintmax_t>(Window_.Start_)
                               + Window_.PreviousRowsetSize_);
            }
            case SQL_FETCH_PRIOR: {
                if (Window_.Position_ == EPosition::Before) {
                    return Boundary(EPosition::Before);
                }
                const size_t origin = Window_.Position_ == EPosition::After
                    ? VisibleRows_
                    : Window_.Start_;
                return Backward(origin, RowsetSize_, origin > 0);
            }
            case SQL_FETCH_FIRST:
                return Rowset(0);
            case SQL_FETCH_LAST:
                return VisibleRows_ == 0
                    ? Boundary(EPosition::After)
                    : Rowset(VisibleRows_ > RowsetSize_
                        ? VisibleRows_ - RowsetSize_
                        : 0);
            case SQL_FETCH_ABSOLUTE:
                return Absolute(offset);
            case SQL_FETCH_RELATIVE: {
                if (Window_.Position_ == EPosition::Before) {
                    return offset > 0 ? Absolute(offset) : Boundary(EPosition::Before);
                }
                if (Window_.Position_ == EPosition::After) {
                    return offset < 0 ? Absolute(offset) : Boundary(EPosition::After);
                }
                if (offset >= 0) {
                    return Positive(static_cast<uintmax_t>(Window_.Start_)
                                    + static_cast<uintmax_t>(offset));
                }
                return Backward(
                    Window_.Start_, NegativeMagnitude(offset), Window_.Start_ > 0);
            }
            default:
                return Boundary(Window_.Position_ == EPosition::After
                    ? EPosition::After
                    : EPosition::Before);
        }
    }

private:
    static TTarget Boundary(EPosition position) {
        return {position};
    }

    static TTarget Rowset(size_t start, bool overlappedStart = false) {
        return {EPosition::Rowset, start, overlappedStart};
    }

    TTarget Positive(uintmax_t start) const {
        return start > std::numeric_limits<size_t>::max()
            ? Boundary(EPosition::After)
            : Rowset(static_cast<size_t>(start));
    }

    TTarget Backward(size_t origin, uintmax_t distance, bool allowOverlap) const {
        if (distance <= origin) {
            return Rowset(origin - static_cast<size_t>(distance));
        }
        return allowOverlap && distance <= RequestedRowsetSize_
            ? Rowset(0, true)
            : Boundary(EPosition::Before);
    }

    TTarget Absolute(SQLLEN offset) const {
        if (offset > 0) {
            return Positive(static_cast<uintmax_t>(offset) - 1);
        }
        if (offset == 0) {
            return Boundary(EPosition::Before);
        }
        return Backward(
            VisibleRows_, NegativeMagnitude(offset), VisibleRows_ > 0);
    }

    const TCursorWindow& Window_;
    size_t RowsetSize_;
    uintmax_t RequestedRowsetSize_;
    size_t VisibleRows_;
};

TFetchResult TCursorWindow::Fetch(
    SQLSMALLINT orientation,
    SQLLEN offset,
    SQLULEN rowsetSize,
    SQLULEN maxRows)
{
    if (rowsetSize == 0) {
        SetBoundary(EPosition::Before);
        return {};
    }

    const size_t rowset = ToSize(rowsetSize);
    const size_t visibleRows = maxRows == 0
        ? TotalRows_
        : std::min(TotalRows_, ToSize(maxRows));
    const auto target = TPositionResolver(*this, rowset, rowsetSize, visibleRows)
        .Resolve(orientation, offset);

    if (target.Position != EPosition::Rowset) {
        SetBoundary(target.Position);
        return {};
    }
    if (target.Start >= visibleRows
        || target.Start > static_cast<size_t>(std::numeric_limits<SQLLEN>::max())) {
        SetBoundary(EPosition::After);
        return {};
    }

    Position_ = EPosition::Rowset;
    Start_ = target.Start;
    Size_ = std::min(rowset, visibleRows - target.Start);
    PreviousRowsetSize_ = rowset;
    return {static_cast<SQLULEN>(Size_), target.OverlappedStart};
}

std::optional<size_t> TCursorWindow::Resolve(SQLULEN row) const {
    if (Position_ != EPosition::Rowset
        || static_cast<uintmax_t>(row) >= Size_) {
        return std::nullopt;
    }
    return Start_ + static_cast<size_t>(row);
}

SQLULEN TCursorWindow::RowNumber() const {
    return Position_ == EPosition::Rowset
        ? static_cast<SQLULEN>(Start_) + 1
        : 0;
}

void TCursorWindow::SetBoundary(EPosition position) {
    Position_ = position;
    Start_ = 0;
    Size_ = 0;
}

TFetchResult ICursor::Fetch(
    SQLSMALLINT orientation,
    SQLLEN offset,
    SQLULEN rowsetSize,
    SQLULEN maxRows)
{
    return Window_.Fetch(orientation, offset, rowsetSize, maxRows);
}

SQLULEN ICursor::GetRowNumber() const {
    return Window_.RowNumber();
}

std::optional<size_t> ICursor::CurrentRow(SQLULEN row) const {
    return Window_.Resolve(row);
}

std::unique_ptr<ICursor> CreateExecCursor(TResultSet resultSet, bool scrollable) {
    if (scrollable) {
        return std::make_unique<TStaticCursor>(resultSet);
    }
    return std::make_unique<TForwardCursor>(resultSet);
}

std::unique_ptr<ICursor> CreateVirtualCursor(TColumnSchema columns, TTable table) {
    return std::make_unique<TVirtualCursor>(columns, std::move(table));
}

} // namespace NYdb::NOdbc
