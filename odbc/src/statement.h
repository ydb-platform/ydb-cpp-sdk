#pragma once

#include "connection.h"
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

    NYdb::TParams BuildParams();
    
    std::vector<NScheme::TSchemeEntry> GetPatternEntries(const std::string& pattern);
    SQLRETURN VisitEntry(const std::string& path, const std::string& pattern, std::vector<NScheme::TSchemeEntry>& resultEntries);
    bool IsPatternMatch(const std::string& path, const std::string& pattern);
    std::optional<std::string> GetTableType(NScheme::ESchemeEntryType type);
};

} // namespace NOdbc
} // namespace NYdb
