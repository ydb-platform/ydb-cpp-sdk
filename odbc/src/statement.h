#pragma once

#include <sql.h>
#include <sqlext.h>
#include <memory>
#include <vector>
#include <string>

#include <ydb-cpp-sdk/client/query/client.h>

#include "connection.h"

namespace NYdb {
namespace NOdbc {

class TStatement {
private:
    TConnection* Conn_;
    TErrorList Errors_;
    std::unique_ptr<NQuery::TExecuteQueryIterator> Iterator_;
    std::unique_ptr<TResultSetParser> ResultSetParser_;
    
    struct TBoundColumn {
        SQLUSMALLINT ColumnNumber;
        SQLSMALLINT TargetType;
        SQLPOINTER TargetValue;
        SQLLEN BufferLength;
        SQLLEN* StrLenOrInd;
    };
    std::vector<TBoundColumn> BoundColumns_;
    
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
    std::vector<TBoundParam> BoundParams_;
    
public:
    TStatement(TConnection* conn);

    SQLRETURN ExecDirect(const std::string& statementText);
    SQLRETURN Fetch();
    SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, 
                     SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd);
    SQLRETURN GetDiagRec(SQLSMALLINT recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeError, 
                        SQLCHAR* messageText, SQLSMALLINT bufferLength, SQLSMALLINT* textLength);
    SQLRETURN BindCol(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd);
    SQLRETURN BindParameter(SQLUSMALLINT paramNumber, SQLSMALLINT inputOutputType, SQLSMALLINT valueType, SQLSMALLINT parameterType, SQLULEN columnSize, SQLSMALLINT decimalDigits, SQLPOINTER parameterValuePtr, SQLLEN bufferLength, SQLLEN* strLenOrIndPtr);
    
    TConnection* GetConnection() { return Conn_; }
    
    void AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message);
    void ClearErrors();

    NYdb::TParams BuildParams();
    
private:
    void ClearStatement();

    SQLRETURN ConvertYdbValue(NYdb::TValueParser& valueParser, SQLSMALLINT targetType, 
                             SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd);
};

} // namespace NOdbc
} // namespace NYdb 