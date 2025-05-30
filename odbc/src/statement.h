#pragma once

#include "connection.h"
#include "utils/result.h"
#include "utils/convert.h"

#include <ydb-cpp-sdk/client/query/client.h>

#include <sql.h>
#include <sqlext.h>

#include <memory>
#include <vector>
#include <string>


namespace NYdb {
namespace NOdbc {

class TStatement {
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

    SQLRETURN Columns(const std::string& catalogName,
                      const std::string& schemaName,
                      const std::string& tableName,
                      const std::string& columnName);

    SQLRETURN Tables(const std::string& catalogName,
                     const std::string& schemaName,
                     const std::string& tableName,
                     const std::string& tableType);

    TConnection* GetConnection() {
        return Conn_;
    }

    void AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message);
    void ClearErrors();

    NYdb::TParams BuildParams();

    void ClearStatement();

private:
    std::vector<NScheme::TSchemeEntry> GetPatternEntries(const std::string& pattern);
    SQLRETURN VisitEntry(const std::string& path, const std::string& pattern, std::vector<NScheme::TSchemeEntry>& resultEntries);
    bool IsPatternMatch(const std::string& path, const std::string& pattern);

    std::optional<std::string> GetTableType(NScheme::ESchemeEntryType type);

    TConnection* Conn_;
    TErrorList Errors_;

    std::unique_ptr<IResultSet> ResultSet_;

    std::vector<TBoundColumn> BoundColumns_;
    std::vector<TBoundParam> BoundParams_;
};

} // namespace NOdbc
} // namespace NYdb
