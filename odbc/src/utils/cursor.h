#pragma once

#include "convert.h"

#include <ydb-cpp-sdk/client/query/client.h>
#include <sql.h>

#include <memory>
#include <span>
#include <string>
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
    virtual bool Fetch() = 0;
    virtual SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                              SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd,
                              SQLLEN* offset = nullptr) = 0;
    const std::vector<TColumnMeta>& GetColumnMeta() const {
        return Columns_;
    }

protected:
    std::vector<TColumnMeta> Columns_;
};

std::unique_ptr<ICursor> CreateExecCursor(const NYdb::NQuery::TExecuteQueryResult& result);

std::unique_ptr<ICursor> CreateVirtualCursor(
    TColumnSchema columns,
    TTable table = {});

} // namespace NYdb::NOdbc
