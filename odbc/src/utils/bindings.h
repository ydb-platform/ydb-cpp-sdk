#pragma once

#include <sql.h>
#include <sqlext.h>

#include <ydb-cpp-sdk/client/types/fwd.h>

#include <string>

namespace NYdb {
namespace NOdbc {

struct TBoundParam {
    SQLUSMALLINT ParamNumber;
    SQLSMALLINT InputOutputType;
    SQLSMALLINT ValueType;
    SQLSMALLINT ParameterType;
    SQLULEN ColumnSize;
    SQLSMALLINT DecimalDigits;
    SQLPOINTER ParameterValuePtr;
    SQLLEN BufferLength;
    SQLLEN* StrLenOrIndPtr;
    bool AtExec = false;
    bool AtExecComplete = false;
    std::string AtExecChunk;
};

} // namespace NOdbc
} // namespace NYdb
