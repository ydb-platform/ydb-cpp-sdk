#include "cursor.h"
#include "error_manager.h"
#include "sql_type_map.h"
#include "types.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

namespace NYdb::NOdbc {

std::vector<TColumnMeta> MakeColumnMeta(const TResultSet& resultSet, bool parameters) {
    std::vector<TColumnMeta> columns;
    columns.reserve(resultSet.GetColumnsMeta().size());
    for (const auto& column : resultSet.GetColumnsMeta()) {
        const TYdbTypeInfo type = DescribeYdbType(column.Type);
        SQLSMALLINT sqlType = type.SqlType;
        const EWideTemporal wide = type.TypeName == "Date32" ? EWideTemporal::Date32
            : type.TypeName == "Datetime64" ? EWideTemporal::Datetime64
            : type.TypeName == "Timestamp64" ? EWideTemporal::Timestamp64 : EWideTemporal::None;
        if (parameters) {
            const auto* spec = FindSqlTypeSpec(sqlType);
            // YDB Datetime contains a date, unlike SQL_TYPE_TIME.
            if (!spec || sqlType == SQL_TYPE_TIME
                || (sqlType != SQL_DECIMAL && spec->YqlType != type.TypeName && wide == EWideTemporal::None)) {
                sqlType = SQL_UNKNOWN_TYPE;
            }
        }
        // YQL qualifies unaliased JOIN projections; dots are forbidden in SQL aliases.
        const size_t dot = column.Name.rfind('.');
        columns.push_back({dot == std::string::npos ? column.Name : column.Name.substr(dot + 1),
                           sqlType, type.ColumnSize, type.Nullable,
                           type.DecimalDigits.value_or(parameters && sqlType == SQL_TYPE_TIMESTAMP
                               && wide != EWideTemporal::Datetime64 ? 6 : 0),
                           type.Unsigned, wide});
    }
    return columns;
}

namespace {

using TResultRow = std::vector<TValue>;
using TResultRows = std::vector<TResultRow>;

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
        const size_t remaining = limit - std::min(RowsRead_, limit);
        CurrentRows_.reserve(std::min(wanted, remaining));
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
                throw TOdbcException(
                    "HY000", 0,
                    "ODBC cursor result ended before its declared row count");
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
    EPosition position = EPosition::Rowset;
    size_t start = 0;
    bool overlappedStart = false;
    const auto forward = [&](size_t origin, uintmax_t distance) {
        if (distance > std::numeric_limits<uintmax_t>::max() - origin
            || static_cast<uintmax_t>(origin) + distance > std::numeric_limits<size_t>::max()) {
            position = EPosition::After;
        } else {
            start = origin + static_cast<size_t>(distance);
        }
    };
    const auto backward = [&](size_t origin, uintmax_t distance, bool allowOverlap) {
        if (distance <= origin) {
            start = origin - static_cast<size_t>(distance);
        } else if (allowOverlap && distance - origin < static_cast<uintmax_t>(rowsetSize)) {
            overlappedStart = true;
        } else {
            position = EPosition::Before;
        }
    };
    const auto absolute = [&] {
        if (offset > 0) {
            forward(0, static_cast<uintmax_t>(offset) - 1);
        } else if (offset == 0) {
            position = EPosition::Before;
        } else {
            backward(visibleRows, NegativeMagnitude(offset), visibleRows > 0);
        }
    };

    switch (orientation) {
        case SQL_FETCH_NEXT:
            if (Position_ == EPosition::After) {
                position = EPosition::After;
            } else if (Position_ == EPosition::Rowset) {
                forward(Start_, PreviousRowsetSize_);
            }
            break;
        case SQL_FETCH_PRIOR:
            if (Position_ == EPosition::Before) {
                position = EPosition::Before;
            } else {
                const size_t origin = Position_ == EPosition::After ? visibleRows : Start_;
                backward(origin, rowset, origin > 0);
            }
            break;
        case SQL_FETCH_FIRST:
            break;
        case SQL_FETCH_LAST:
            if (visibleRows == 0) {
                position = EPosition::After;
            } else {
                start = visibleRows > rowset ? visibleRows - rowset : 0;
            }
            break;
        case SQL_FETCH_ABSOLUTE:
            absolute();
            break;
        case SQL_FETCH_RELATIVE:
            if (Position_ != EPosition::Rowset) {
                if ((Position_ == EPosition::Before && offset > 0)
                    || (Position_ == EPosition::After && offset < 0)) {
                    absolute();
                } else {
                    position = Position_;
                }
            } else if (offset >= 0) {
                forward(Start_, static_cast<uintmax_t>(offset));
            } else {
                backward(Start_, NegativeMagnitude(offset), Start_ > 0);
            }
            break;
        default:
            position = Position_ == EPosition::After ? EPosition::After : EPosition::Before;
    }

    if (position != EPosition::Rowset) {
        SetBoundary(position);
        return {};
    }
    if (start >= visibleRows
        || start > static_cast<size_t>(std::numeric_limits<SQLLEN>::max())) {
        SetBoundary(EPosition::After);
        return {};
    }

    Position_ = EPosition::Rowset;
    Start_ = start;
    Size_ = std::min(rowset, visibleRows - start);
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
