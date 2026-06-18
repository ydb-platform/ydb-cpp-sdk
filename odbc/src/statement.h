#pragma once

#include "connection.h"
#include "statement_attr.h"
#include "descriptor.h"
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
    friend class TDescriptor;
public:
    TStatement(TConnection* conn);

    SQLRETURN Prepare(const std::string& statementText);
    SQLRETURN Execute();
    SQLRETURN ExecuteInternal();

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

    SQLRETURN GetTypeInfo(SQLSMALLINT dataType);
    SQLRETURN Statistics(const std::string& catalogName,
                         const std::string& schemaName,
                         const std::string& tableName,
                         SQLUSMALLINT unique,
                         SQLUSMALLINT accuracy);
    SQLRETURN SpecialColumns(const std::string& catalogName,
                             const std::string& schemaName,
                             const std::string& tableName,
                             SQLUSMALLINT identifierType,
                             SQLUSMALLINT scope);
    SQLRETURN PrimaryKeys(const std::string& catalogName,
                          const std::string& schemaName,
                          const std::string& tableName);
    SQLRETURN ForeignKeys(const std::string& pkCatalogName,
                          const std::string& pkSchemaName,
                          const std::string& pkTableName,
                          const std::string& fkCatalogName,
                          const std::string& fkSchemaName,
                          const std::string& fkTableName);
    SQLRETURN NumParams(SQLSMALLINT* paramCount);
    SQLRETURN DescribeParam(SQLUSMALLINT paramNumber, SQLSMALLINT* dataTypePtr, SQLULEN* paramSizePtr,
                            SQLSMALLINT* decimalDigitsPtr, SQLSMALLINT* nullablePtr);
    SQLRETURN ParamData(SQLPOINTER* valuePtr);
    SQLRETURN PutData(SQLPOINTER data, SQLLEN strLenOrInd);
    SQLRETURN Cancel();
    SQLRETURN SetCursorName(const std::string& name);
    SQLRETURN GetCursorName(SQLCHAR* name, SQLSMALLINT bufferLength, SQLSMALLINT* nameLengthPtr);

    TDescriptor& GetAppRowDesc() { return *AppRowDesc_; }
    TDescriptor& GetAppParamDesc() { return *AppParamDesc_; }
    TDescriptor& GetImpRowDesc() { return *ImpRowDesc_; }
    TDescriptor& GetImpParamDesc() { return *ImpParamDesc_; }

    SQLRETURN RowCount(SQLLEN* rowCount);
    SQLRETURN NumResultCols(SQLSMALLINT* colCount);
    const std::vector<TColumnMeta>& GetColumnMeta() const;
    SQLRETURN SetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER stringLength);
    SQLRETURN GetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr);

    SQLRETURN GetDiagField(SQLSMALLINT recNumber, SQLSMALLINT diagIdentifier, SQLPOINTER diagInfoPtr, SQLSMALLINT bufferLength,
        SQLSMALLINT* stringLengthPtr) override;

    SQLSMALLINT GetParamCount() const { return ParamCount_; }

    TConnection* GetConnection() {
        return Conn_;
    }

private:
    TConnection* Conn_;
    std::unique_ptr<ICursor> Cursor_;
    std::string PreparedQuery_;
    bool IsPrepared_ = false;
    SQLSMALLINT ParamCount_ = 0;

    std::vector<TBoundColumn> BoundColumns_;
    std::vector<TBoundParam> BoundParams_;
    bool StreamFetchError_ = false;
    SQLULEN RowsFetched_ = 0;
    SQLLEN RowCount_ = -1;
    TStatementAttributes Attributes_;
    std::string CursorName_;
    std::unique_ptr<TDescriptor> AppRowDesc_;
    std::unique_ptr<TDescriptor> AppParamDesc_;
    std::unique_ptr<TDescriptor> ImpRowDesc_;
    std::unique_ptr<TDescriptor> ImpParamDesc_;
    SQLUSMALLINT NeedDataParam_ = 0;
    bool InAtExec_ = false;
    SQLRETURN LastFetchRc_ = SQL_SUCCESS;

    SQLRETURN BuildParams(NYdb::TParams& out);

    void ResetForMetadata();

    SQLUSMALLINT FindNextNeedDataParam() const;
    std::string GetTraversalRoot(const std::string& pattern) const;

    NQuery::TExecuteQueryIterator CreateExecuteIterator(NQuery::TSession& session, const NYdb::TParams& params);

    NYdb::NRetry::TRetryOperationSettings MakeAutocommitRetrySettings();
    std::vector<NScheme::TSchemeEntry> GetPatternEntries(const std::string& pattern);
    SQLRETURN VisitEntry(const std::string& path, const std::string& pattern, std::vector<NScheme::TSchemeEntry>& resultEntries);
    bool IsPatternMatch(const std::string& path, const std::string& pattern);
    std::optional<std::string> GetTableType(NScheme::ESchemeEntryType type);
};

} // namespace NOdbc
} // namespace NYdb
