#pragma once

#include <sql.h>
#include <sqlext.h>

#include <ydb-cpp-sdk/client/types/status/status.h>

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
    virtual void OnStreamPartError(const TStatus& status) {
        (void)status;
    }

    virtual ~IBindingFiller() = default;
};

} // namespace NOdbc
} // namespace NYdb
