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

struct TBoundColumn {
    SQLUSMALLINT ColumnNumber;
    SQLSMALLINT TargetType;
    SQLPOINTER TargetValue;
    SQLLEN BufferLength;
    SQLLEN* StrLenOrInd;
};

class IBindingFiller {
public:
    virtual void FillBoundColumns() = 0;
    virtual void OnStreamPartError([[maybe_unused]] const TStatus& status) {
    }

    virtual ~IBindingFiller() = default;
};

} // namespace NOdbc
} // namespace NYdb
