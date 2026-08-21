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
    std::optional<size_t> target;
    std::optional<EPosition> boundary;
    bool overlappedStart = false;

    const auto setPositiveTarget = [&](uintmax_t index) {
        if (index > std::numeric_limits<size_t>::max()) {
            boundary = EPosition::After;
        } else {
            target = static_cast<size_t>(index);
        }
    };
    const auto setAbsoluteTarget = [&](SQLLEN absoluteOffset) {
        if (absoluteOffset > 0) {
            setPositiveTarget(static_cast<uintmax_t>(absoluteOffset) - 1);
            return;
        }
        if (absoluteOffset == 0) {
            boundary = EPosition::Before;
            return;
        }

        const uintmax_t magnitude = NegativeMagnitude(absoluteOffset);
        if (magnitude <= visibleRows) {
            target = visibleRows - static_cast<size_t>(magnitude);
        } else if (visibleRows > 0 && magnitude <= static_cast<uintmax_t>(rowsetSize)) {
            target = 0;
            overlappedStart = true;
        } else {
            boundary = EPosition::Before;
        }
    };

    switch (orientation) {
        case SQL_FETCH_NEXT:
            if (Position_ == EPosition::Before) {
                target = 0;
            } else if (Position_ == EPosition::After) {
                boundary = EPosition::After;
            } else {
                setPositiveTarget(
                    static_cast<uintmax_t>(Start_) + PreviousRowsetSize_);
            }
            break;

        case SQL_FETCH_PRIOR:
            if (Position_ == EPosition::Before) {
                boundary = EPosition::Before;
            } else if (Position_ == EPosition::After) {
                if (visibleRows == 0) {
                    boundary = EPosition::Before;
                } else if (visibleRows < rowset) {
                    target = 0;
                    overlappedStart = true;
                } else {
                    target = visibleRows - rowset;
                }
            } else if (Start_ == 0) {
                boundary = EPosition::Before;
            } else if (Start_ < rowset) {
                target = 0;
                overlappedStart = true;
            } else {
                target = Start_ - rowset;
            }
            break;

        case SQL_FETCH_FIRST:
            target = 0;
            break;

        case SQL_FETCH_LAST:
            if (visibleRows == 0) {
                boundary = EPosition::After;
            } else if (rowset >= visibleRows) {
                target = 0;
            } else {
                target = visibleRows - rowset;
            }
            break;

        case SQL_FETCH_ABSOLUTE:
            setAbsoluteTarget(offset);
            break;

        case SQL_FETCH_RELATIVE:
            if (Position_ == EPosition::Before) {
                if (offset > 0) {
                    setAbsoluteTarget(offset);
                } else {
                    boundary = EPosition::Before;
                }
            } else if (Position_ == EPosition::After) {
                if (offset < 0) {
                    setAbsoluteTarget(offset);
                } else {
                    boundary = EPosition::After;
                }
            } else if (offset >= 0) {
                setPositiveTarget(static_cast<uintmax_t>(Start_)
                                  + static_cast<uintmax_t>(offset));
            } else if (Start_ == 0) {
                boundary = EPosition::Before;
            } else {
                const uintmax_t magnitude = NegativeMagnitude(offset);
                if (magnitude <= Start_) {
                    target = Start_ - static_cast<size_t>(magnitude);
                } else if (magnitude <= static_cast<uintmax_t>(rowsetSize)) {
                    target = 0;
                    overlappedStart = true;
                } else {
                    boundary = EPosition::Before;
                }
            }
            break;

        default:
            boundary = Position_ == EPosition::After
                ? EPosition::After
                : EPosition::Before;
            break;
    }

    if (boundary) {
        SetBoundary(*boundary);
        return {};
    }
    if (!target || *target >= visibleRows
        || *target > static_cast<size_t>(std::numeric_limits<SQLLEN>::max())) {
        SetBoundary(EPosition::After);
        return {};
    }

    Position_ = EPosition::Rowset;
    Start_ = *target;
    Size_ = std::min(rowset, visibleRows - *target);
    PreviousRowsetSize_ = rowset;
    return {static_cast<SQLULEN>(Size_), overlappedStart};
}

std::optional<size_t> TCursorWindow::Resolve(SQLULEN row) const {
    if (Position_ != EPosition::Rowset
        || static_cast<uintmax_t>(row) >= Size_) {
        return std::nullopt;
    }
    return Start_ + static_cast<size_t>(row);
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
