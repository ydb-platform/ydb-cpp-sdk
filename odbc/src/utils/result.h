#pragma once

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

class IResultSet {
public:
    virtual ~IResultSet() = default;    
    virtual bool Fetch() = 0;
    virtual SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                              SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) = 0;
    virtual SQLRETURN BindCol(SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
                              SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) = 0;
    virtual size_t ColumnsCount() const = 0;
    virtual const TColumnMeta& GetColumnMeta(size_t index) const = 0;
};

std::unique_ptr<IResultSet> CreateExecResultSet(NYdb::NQuery::TExecuteQueryIterator iterator);
std::unique_ptr<IResultSet> CreateVirtualResultSet(const std::vector<TColumnMeta>& columns, const TTable& table);

} // namespace NOdbc
} // namespace NYdb
