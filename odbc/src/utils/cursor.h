#pragma once

#include "convert.h"

#include <ydb-cpp-sdk/client/result/result.h>
#include <sql.h>

#include <memory>
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

class ICursor {
public:
    virtual ~ICursor() = default;
    virtual TFetchResult Fetch(SQLSMALLINT orientation, SQLLEN offset,
                               SQLULEN rowsetSize, SQLULEN maxRows) = 0;
    virtual SQLRETURN GetData(SQLULEN row, SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                              SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd,
                              SQLLEN* offset = nullptr) = 0;
    const std::vector<TColumnMeta>& GetColumnMeta() const {
        return Columns_;
    }

protected:
    explicit ICursor(std::vector<TColumnMeta> columns = {})
        : Columns_(std::move(columns))
    {}

    std::vector<TColumnMeta> Columns_;
};

std::unique_ptr<ICursor> CreateExecCursor(TResultSet resultSet, bool scrollable);

std::unique_ptr<ICursor> CreateVirtualCursor(
    TColumnSchema columns,
    TTable table = {});

} // namespace NYdb::NOdbc
