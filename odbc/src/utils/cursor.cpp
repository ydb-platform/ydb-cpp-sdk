#include "cursor.h"
#include "types.h"

#include <ydb-cpp-sdk/client/result/rows.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <limits>
#include <mutex>
#include <optional>
#include <shared_mutex>
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

TResultRow MaterializeRow(TRowParser& row) {
    TResultRow result;
    result.reserve(row.ColumnsCount());
    for (size_t column = 0; column < row.ColumnsCount(); ++column) {
        result.push_back(row.GetValue(column));
    }
    return result;
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
    explicit TForwardCursor(TResultSet resultSet)
        : ICursor(MakeColumnMeta(resultSet))
        , Range_(std::move(resultSet))
        , Iterator_(Range_.begin())
    {}

    TFetchResult Fetch(SQLSMALLINT orientation, SQLLEN, SQLULEN rowsetSize,
                       SQLULEN maxRows) override {
        CurrentRows_.clear();
        if (orientation != SQL_FETCH_NEXT || rowsetSize == 0) {
            return {};
        }

        const size_t wanted = ToSize(rowsetSize);
        const size_t limit = maxRows == 0
            ? std::numeric_limits<size_t>::max()
            : ToSize(maxRows);
        while (CurrentRows_.size() < wanted && RowsRead_ < limit
               && Iterator_ != Range_.end()) {
            CurrentRows_.push_back(MaterializeRow(*Iterator_));
            ++Iterator_;
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
    TRowRange Range_;
    TRowIterator Iterator_;
    TResultRows CurrentRows_;
    size_t RowsRead_ = 0;
};

class TScrollableCursor : public ICursor {
public:
    TFetchResult Fetch(SQLSMALLINT orientation, SQLLEN offset, SQLULEN rowsetSize,
                       SQLULEN maxRows) final {
        std::unique_lock lock(Mutex_);
        if (rowsetSize == 0) {
            SetBoundary(kBeforeFirst);
            return {};
        }

        const size_t rowset = ToSize(rowsetSize);
        const size_t limit = maxRows == 0
            ? std::numeric_limits<size_t>::max()
            : ToSize(maxRows);
        const SQLLEN current = CurrentStart_.load(std::memory_order_relaxed);
        std::optional<size_t> target;
        SQLLEN boundary = 0;
        bool hasBoundary = false;
        bool overlappedStart = false;

        const auto setBoundary = [&](SQLLEN value) {
            boundary = value;
            hasBoundary = true;
            target.reset();
        };
        const auto materializedEnd = [&]() {
            MaterializeAllUnlocked(limit);
            return std::min(MaterializedRowsUnlocked(), limit);
        };
        const auto setPositiveTarget = [&](uintmax_t index) {
            if (index > std::numeric_limits<size_t>::max()) {
                setBoundary(kAfterLast);
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
                setBoundary(kBeforeFirst);
                return;
            }

            const size_t rows = materializedEnd();
            const uintmax_t magnitude = NegativeMagnitude(absoluteOffset);
            if (magnitude <= rows) {
                target = rows - static_cast<size_t>(magnitude);
            } else if (rows > 0 && magnitude <= static_cast<uintmax_t>(rowsetSize)) {
                target = 0;
                overlappedStart = true;
            } else {
                setBoundary(kBeforeFirst);
            }
        };

        switch (orientation) {
            case SQL_FETCH_NEXT:
                if (current == kBeforeFirst) {
                    target = 0;
                } else if (current == kAfterLast) {
                    setBoundary(kAfterLast);
                } else {
                    const uintmax_t next = static_cast<uintmax_t>(current)
                        + static_cast<uintmax_t>(PreviousRowsetSize_);
                    setPositiveTarget(next);
                }
                break;

            case SQL_FETCH_PRIOR:
                if (current == kBeforeFirst) {
                    setBoundary(kBeforeFirst);
                } else if (current == kAfterLast) {
                    const size_t rows = materializedEnd();
                    if (rows == 0) {
                        setBoundary(kBeforeFirst);
                    } else if (static_cast<uintmax_t>(rows)
                               < static_cast<uintmax_t>(rowsetSize)) {
                        target = 0;
                        overlappedStart = true;
                    } else {
                        target = rows - rowset;
                    }
                } else if (current == 0) {
                    setBoundary(kBeforeFirst);
                } else if (static_cast<uintmax_t>(current)
                           < static_cast<uintmax_t>(rowsetSize)) {
                    target = 0;
                    overlappedStart = true;
                } else {
                    target = static_cast<size_t>(current) - rowset;
                }
                break;

            case SQL_FETCH_FIRST:
                target = 0;
                break;

            case SQL_FETCH_LAST: {
                const size_t rows = materializedEnd();
                if (rows == 0) {
                    setBoundary(kAfterLast);
                } else if (static_cast<uintmax_t>(rowsetSize) >= rows) {
                    target = 0;
                } else {
                    target = rows - rowset;
                }
                break;
            }

            case SQL_FETCH_ABSOLUTE:
                setAbsoluteTarget(offset);
                break;

            case SQL_FETCH_RELATIVE:
                if (current == kBeforeFirst) {
                    if (offset > 0) {
                        setAbsoluteTarget(offset);
                    } else {
                        setBoundary(kBeforeFirst);
                    }
                } else if (current == kAfterLast) {
                    if (offset < 0) {
                        setAbsoluteTarget(offset);
                    } else {
                        setBoundary(kAfterLast);
                    }
                } else if (offset >= 0) {
                    const uintmax_t next = static_cast<uintmax_t>(current)
                        + static_cast<uintmax_t>(offset);
                    setPositiveTarget(next);
                } else if (current == 0) {
                    setBoundary(kBeforeFirst);
                } else {
                    const uintmax_t magnitude = NegativeMagnitude(offset);
                    if (magnitude <= static_cast<uintmax_t>(current)) {
                        target = static_cast<size_t>(current)
                            - static_cast<size_t>(magnitude);
                    } else if (magnitude <= static_cast<uintmax_t>(rowsetSize)) {
                        target = 0;
                        overlappedStart = true;
                    } else {
                        setBoundary(kBeforeFirst);
                    }
                }
                break;

            default:
                setBoundary(current == kAfterLast ? kAfterLast : kBeforeFirst);
                break;
        }

        if (hasBoundary) {
            SetBoundary(boundary);
            return {};
        }
        if (!target || *target >= limit
            || *target > static_cast<size_t>(std::numeric_limits<SQLLEN>::max())) {
            SetBoundary(kAfterLast);
            return {};
        }

        EnsureRowsUnlocked(*target + 1, limit);
        size_t available = std::min(MaterializedRowsUnlocked(), limit);
        if (*target >= available) {
            SetBoundary(kAfterLast);
            return {};
        }

        const size_t wanted = std::min(rowset, limit - *target);
        EnsureRowsUnlocked(*target + wanted, limit);
        available = std::min(MaterializedRowsUnlocked(), limit);
        const size_t rows = std::min(wanted, available - *target);

        CurrentStart_.store(static_cast<SQLLEN>(*target), std::memory_order_release);
        CurrentRowCount_ = rows;
        PreviousRowsetSize_ = rowsetSize;
        return {static_cast<SQLULEN>(rows), overlappedStart};
    }

protected:
    explicit TScrollableCursor(std::vector<TColumnMeta> columns)
        : ICursor(std::move(columns))
    {}

    std::optional<size_t> CurrentRowUnlocked(SQLULEN row) const {
        if (static_cast<uintmax_t>(row) >= CurrentRowCount_) {
            return std::nullopt;
        }
        const SQLLEN current = CurrentStart_.load(std::memory_order_acquire);
        if (current < 0) {
            return std::nullopt;
        }
        const uintmax_t index = static_cast<uintmax_t>(current)
            + static_cast<uintmax_t>(row);
        if (index > std::numeric_limits<size_t>::max()) {
            return std::nullopt;
        }
        return static_cast<size_t>(index);
    }

    virtual void EnsureRowsUnlocked(size_t count, size_t limit) = 0;
    virtual void MaterializeAllUnlocked(size_t limit) = 0;
    virtual size_t MaterializedRowsUnlocked() const = 0;

    mutable std::shared_mutex Mutex_;

private:
    void SetBoundary(SQLLEN boundary) {
        CurrentStart_.store(boundary, std::memory_order_release);
        CurrentRowCount_ = 0;
    }

    static constexpr SQLLEN kBeforeFirst = -1;
    static constexpr SQLLEN kAfterLast = -2;

    std::atomic<SQLLEN> CurrentStart_{kBeforeFirst};
    size_t CurrentRowCount_ = 0;
    SQLULEN PreviousRowsetSize_ = 0;
};

class TStaticCursor final : public TScrollableCursor {
public:
    explicit TStaticCursor(TResultSet resultSet)
        : TScrollableCursor(MakeColumnMeta(resultSet))
        , Range_(std::move(resultSet))
        , Iterator_(Range_.begin())
    {}

    SQLRETURN GetData(SQLULEN row, SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                      SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd,
                      SQLLEN* offset) override {
        std::shared_lock lock(Mutex_);
        const auto rowIndex = CurrentRowUnlocked(row);
        if (!rowIndex || *rowIndex >= Rows_.size()) {
            return SQL_NO_DATA;
        }
        if (columnNumber < 1 || columnNumber > Columns_.size()) {
            return SQL_ERROR;
        }
        TValueParser parser(Rows_[*rowIndex][columnNumber - 1]);
        return ConvertColumn(
            parser, targetType, targetValue, bufferLength, strLenOrInd, offset);
    }

private:
    void EnsureRowsUnlocked(size_t count, size_t limit) override {
        const size_t target = std::min(count, limit);
        while (Rows_.size() < target && !SourceExhausted_) {
            if (Iterator_ == Range_.end()) {
                SourceExhausted_ = true;
                break;
            }
            Rows_.push_back(MaterializeRow(*Iterator_));
            ++Iterator_;
        }
    }

    void MaterializeAllUnlocked(size_t limit) override {
        EnsureRowsUnlocked(limit, limit);
    }

    size_t MaterializedRowsUnlocked() const override {
        return Rows_.size();
    }

    TRowRange Range_;
    TRowIterator Iterator_;
    TResultRows Rows_;
    bool SourceExhausted_ = false;
};

class TVirtualCursor final : public TScrollableCursor {
public:
    TVirtualCursor(TColumnSchema columns, TTable table)
        : TScrollableCursor(std::vector<TColumnMeta>(columns.begin(), columns.end()))
        , Table_(std::move(table))
    {}

    SQLRETURN GetData(SQLULEN row, SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                      SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd,
                      SQLLEN* offset) override {
        std::shared_lock lock(Mutex_);
        const auto rowIndex = CurrentRowUnlocked(row);
        if (!rowIndex || *rowIndex >= Table_.size()) {
            return SQL_NO_DATA;
        }
        if (columnNumber < 1 || columnNumber > Columns_.size()) {
            return SQL_ERROR;
        }
        return ConvertColumn(Table_[*rowIndex][columnNumber - 1], targetType,
                             targetValue, bufferLength, strLenOrInd, offset);
    }

private:
    void EnsureRowsUnlocked(size_t, size_t) override {
    }

    void MaterializeAllUnlocked(size_t) override {
    }

    size_t MaterializedRowsUnlocked() const override {
        return Table_.size();
    }

    TTable Table_;
};

} // namespace

std::unique_ptr<ICursor> CreateExecCursor(TResultSet resultSet, bool scrollable) {
    if (scrollable) {
        return std::make_unique<TStaticCursor>(std::move(resultSet));
    }
    return std::make_unique<TForwardCursor>(std::move(resultSet));
}

std::unique_ptr<ICursor> CreateVirtualCursor(TColumnSchema columns, TTable table) {
    return std::make_unique<TVirtualCursor>(columns, std::move(table));
}

} // namespace NYdb::NOdbc
