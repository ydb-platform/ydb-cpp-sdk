#pragma once

#include "convert.h"
#include "cursor_window.h"

#include <ydb-cpp-sdk/client/result/result.h>
#include "odbc_compat.h"

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

class ICursor {
public:
    virtual ~ICursor() = default;
    virtual TFetchResult Fetch(
        SQLSMALLINT orientation,
        SQLLEN offset,
        SQLULEN rowsetSize,
        SQLULEN maxRows);
    virtual SQLULEN GetRowNumber() const;
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
