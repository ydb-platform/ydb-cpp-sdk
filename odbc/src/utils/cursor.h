#pragma once

#include "bindings.h"

#include <ydb-cpp-sdk/client/query/client.h>

#include <sql.h>

#include <string>
#include <vector>

namespace NYdb {
namespace NOdbc {

struct TColumnMeta {
    std::string Name;
    SQLSMALLINT SqlType;
    SQLULEN Size;
    SQLSMALLINT Nullable;
};

using TTable = std::vector<std::vector<TValue>>;

class ICursor {
public:
    virtual ~ICursor() = default;    
    virtual bool Fetch() = 0;
    virtual SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                              SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) = 0;
    virtual const std::vector<TColumnMeta>& GetColumnMeta() const = 0;
};

std::unique_ptr<ICursor> CreateExecCursor(IBindingFiller* bindingFiller, NYdb::NQuery::TExecuteQueryIterator iterator);
std::unique_ptr<ICursor> CreateVirtualCursor(IBindingFiller* bindingFiller, const std::vector<TColumnMeta>& columns, const TTable& table);

} // namespace NOdbc
} // namespace NYdb
