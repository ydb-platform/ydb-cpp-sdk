#pragma once

#include "connection.h"
#include "statement_attr.h"
#include "utils/error_manager.h"
#include "utils/bindings.h"
#include "utils/cursor.h"

#include <ydb-cpp-sdk/client/query/client.h>

#include <sql.h>
#include <sqlext.h>

#include <memory>
#include <vector>
#include <string>


namespace NYdb {
namespace NOdbc {

class TStatement : public TErrorManager, public IBindingFiller {
public:
    TStatement(TConnection* conn);

    SQLRETURN Prepare(const std::string& statementText);
    SQLRETURN Execute();

    SQLRETURN Fetch();
    SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, 
                     SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd);

    void FillBoundColumns() override;
    void OnStreamPartError(const TStatus& status) override;

    SQLRETURN Close(bool force = false);
    void UnbindColumns();
    void ResetParams();

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

    SQLRETURN RowCount(SQLLEN* rowCount);
    SQLRETURN NumResultCols(SQLSMALLINT* colCount);
    SQLRETURN SetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER stringLength);
    SQLRETURN GetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr);

    SQLRETURN GetDiagField(SQLSMALLINT recNumber, SQLSMALLINT diagIdentifier, SQLPOINTER diagInfoPtr, SQLSMALLINT bufferLength,
        SQLSMALLINT* stringLengthPtr) override;

    TConnection* GetConnection() {
        return Conn_;
    }

private:
    TConnection* Conn_;
    std::unique_ptr<ICursor> Cursor_;
    std::string PreparedQuery_;
    bool IsPrepared_ = false;

    std::vector<TBoundColumn> BoundColumns_;
    std::vector<TBoundParam> BoundParams_;
    bool StreamFetchError_ = false;
    SQLULEN RowsFetched_ = 0;
    TStatementAttributes Attributes_;

    NYdb::TParams BuildParams();
    
    NQuery::TExecuteQueryIterator CreateExecuteIterator(NQuery::TSession& session, const NYdb::TParams& params);

    NYdb::NRetry::TRetryOperationSettings MakeAutocommitRetrySettings();
    std::vector<NScheme::TSchemeEntry> GetPatternEntries(const std::string& pattern);
    SQLRETURN VisitEntry(const std::string& path, const std::string& pattern, std::vector<NScheme::TSchemeEntry>& resultEntries);
    bool IsPatternMatch(const std::string& path, const std::string& pattern);
    std::optional<std::string> GetTableType(NScheme::ESchemeEntryType type);
};

} // namespace NOdbc
} // namespace NYdb
