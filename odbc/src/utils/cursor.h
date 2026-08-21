#pragma once

#include "convert.h"

#include <ydb-cpp-sdk/client/result/result.h>
#include <sql.h>

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace NYdb::NOdbc {

struct TColumnMeta {
    std::string Name;
    SQLSMALLINT SqlType;
    SQLULEN Size;
    SQLSMALLINT Nullable;
    SQLSMALLINT DecimalDigits = 0;
    bool Unsigned = false;
};

using TColumnSchema = std::span<const TColumnMeta>;
using TTable = std::vector<std::vector<TOdbcScalar>>;

struct TFetchResult {
    SQLULEN Rows = 0;
    bool OverlappedStart = false;
};

class TCursorWindow {
public:
    explicit TCursorWindow(size_t totalRows = 0);

    TFetchResult Fetch(
        SQLSMALLINT orientation,
        SQLLEN offset,
        SQLULEN rowsetSize,
        SQLULEN maxRows);
    std::optional<size_t> Resolve(SQLULEN row) const;

private:
    enum class EPosition {
        Before,
        Rowset,
        After,
    };

    void SetBoundary(EPosition position);

    size_t TotalRows_ = 0;
    EPosition Position_ = EPosition::Before;
    size_t Start_ = 0;
    size_t Size_ = 0;
    size_t PreviousRowsetSize_ = 0;
};

class ICursor {
public:
    virtual ~ICursor() = default;
    virtual TFetchResult Fetch(
        SQLSMALLINT orientation,
        SQLLEN offset,
        SQLULEN rowsetSize,
        SQLULEN maxRows);
    virtual SQLRETURN GetData(SQLULEN row, SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                              SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd,
                              SQLLEN* offset = nullptr) = 0;
    const std::vector<TColumnMeta>& GetColumnMeta() const {
        return Columns_;
    }

protected:
    explicit ICursor(std::vector<TColumnMeta> columns = {}, size_t rows = 0)
        : Columns_(std::move(columns))
        , Window_(rows)
    {}

    std::optional<size_t> CurrentRow(SQLULEN row) const;

    std::vector<TColumnMeta> Columns_;

private:
    TCursorWindow Window_;
};

std::unique_ptr<ICursor> CreateExecCursor(TResultSet resultSet, bool scrollable);

std::unique_ptr<ICursor> CreateVirtualCursor(
    TColumnSchema columns,
    TTable table = {});

} // namespace NYdb::NOdbc
