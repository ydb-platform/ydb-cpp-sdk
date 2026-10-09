#pragma once

#include "connection.h"
#include "descriptor.h"
#include "utils/attr.h"
#include "utils/bindings.h"
#include "utils/cursor.h"

#include <ydb-cpp-sdk/client/query/client.h>

#include "odbc_compat.h"

#include <memory>
#include <deque>
#include <optional>
#include <vector>
#include <string>
#include <string_view>


namespace NYdb::NOdbc {

using TMetadataArgument = std::optional<std::string>;

class TStatement : public THandle {
public:
    explicit TStatement(std::shared_ptr<TConnection> conn);
    TConnection& GetConnection() const noexcept { return static_cast<TConnection&>(*GetParent()); }
    void RegisterDescriptors(const std::shared_ptr<TStatement>& owner);
    void UnregisterDescriptors();
    void BeforeCall() override;

    SQLRETURN Prepare(const std::string& statementText);
    SQLRETURN Execute();
    SQLRETURN MoreResults();

    SQLRETURN Fetch();
    SQLRETURN FetchScroll(SQLSMALLINT orientation, SQLLEN offset);
    SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, 
                     SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd);

    SQLRETURN Close(bool force = false);
    void UnbindColumns();
    void ResetParams();

    SQLRETURN BindCol(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd);
    SQLRETURN BindParameter(SQLUSMALLINT paramNumber, SQLSMALLINT inputOutputType, SQLSMALLINT valueType, SQLSMALLINT parameterType, SQLULEN columnSize, SQLSMALLINT decimalDigits, SQLPOINTER parameterValuePtr, SQLLEN bufferLength, SQLLEN* strLenOrIndPtr);

    SQLRETURN Columns(const TMetadataArgument& catalogName, const TMetadataArgument& schemaName,
                      const TMetadataArgument& tableName, const TMetadataArgument& columnName);

    SQLRETURN Tables(const TMetadataArgument& catalogName, const TMetadataArgument& schemaName,
                     const TMetadataArgument& tableName, const TMetadataArgument& tableType);

    SQLRETURN GetTypeInfo(SQLSMALLINT dataType);
    SQLRETURN Statistics(const TMetadataArgument& catalogName,
                         const TMetadataArgument& schemaName,
                         const TMetadataArgument& tableName,
                         SQLUSMALLINT unique,
                         SQLUSMALLINT accuracy);
    SQLRETURN SpecialColumns(const TMetadataArgument& catalogName,
                             const TMetadataArgument& schemaName,
                             const TMetadataArgument& tableName,
                             SQLUSMALLINT identifierType,
                             SQLUSMALLINT scope);
    SQLRETURN PrimaryKeys(const TMetadataArgument& catalogName, const TMetadataArgument& schemaName,
                          const TMetadataArgument& tableName);
    SQLRETURN ForeignKeys(const std::string& pkCatalogName,
                          const std::string& pkSchemaName,
                          const std::string& pkTableName,
                          const std::string& fkCatalogName,
                          const std::string& fkSchemaName,
                          const std::string& fkTableName);
    SQLRETURN ColumnPrivileges(const std::string& catalogName,
                               const std::string& schemaName,
                               const std::string& tableName,
                               const std::string& columnName);
    SQLRETURN NumParams(SQLSMALLINT* paramCount);
    SQLRETURN DescribeParam(SQLUSMALLINT paramNumber, SQLSMALLINT* dataTypePtr, SQLULEN* paramSizePtr,
                            SQLSMALLINT* decimalDigitsPtr, SQLSMALLINT* nullablePtr);
    SQLRETURN ParamData(SQLPOINTER* valuePtr);
    SQLRETURN PutData(SQLPOINTER data, SQLLEN strLenOrInd);
    SQLRETURN Cancel();
    bool IsExecuting() const { return ExecutionState_.load(std::memory_order_relaxed) != EExecutionState::Idle; }
    bool IsCancelRequested() const { return ExecutionState_.load(std::memory_order_relaxed) == EExecutionState::CancelRequested; }
    SQLRETURN SetCursorName(const std::string& name);
    SQLRETURN GetCursorName(SQLCHAR* name, SQLSMALLINT bufferLength, SQLSMALLINT* nameLengthPtr);


    SQLRETURN RowCount(SQLLEN* rowCount);
    SQLRETURN NumResultCols(SQLSMALLINT* colCount);
    const std::vector<TColumnMeta>& GetColumnMeta();
    SQLRETURN SetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER stringLength);
    SQLRETURN GetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr);

    SQLRETURN GetDiagField(SQLSMALLINT recNumber, SQLSMALLINT diagIdentifier, SQLPOINTER diagInfoPtr, SQLSMALLINT bufferLength,
        SQLSMALLINT* stringLengthPtr) override;

private:
    friend class TConnection;

    enum class EExecutionState : unsigned char { Idle, Executing, CancelRequested };

    struct TAttributes {
        SQLUINTEGER QueryTimeoutSec = 0;
        SQLULEN MaxRows = 0;
        SQLULEN NoScan = SQL_NOSCAN_OFF;
        SQLULEN MetadataId = SQL_FALSE;
        SQLULEN CursorType = SQL_CURSOR_FORWARD_ONLY;
        SQLULEN Concurrency = SQL_CONCUR_READ_ONLY;

        SQLUINTEGER GetQueryTimeoutSec() const noexcept { return QueryTimeoutSec; }
        SQLULEN GetMaxRows() const noexcept { return MaxRows; }
        SQLULEN GetNoScanMode() const noexcept { return NoScan; }
        SQLULEN GetMetadataId() const noexcept { return MetadataId; }
    };

    using TDirectAttributes = TScalarProperties<
        TScalarProperty<SQL_ATTR_QUERY_TIMEOUT, &TAttributes::QueryTimeoutSec>,
        TScalarProperty<SQL_ATTR_MAX_ROWS, &TAttributes::MaxRows>,
        TScalarProperty<SQL_ATTR_NOSCAN, &TAttributes::NoScan>,
        TScalarProperty<SQL_ATTR_METADATA_ID, &TAttributes::MetadataId>,
        TScalarProperty<SQL_ATTR_CURSOR_TYPE, &TAttributes::CursorType>,
        TScalarProperty<SQL_ATTR_CONCURRENCY, &TAttributes::Concurrency>>;

    struct TAtExecValue {
        std::string Data;
        SQLLEN Indicator = 0;
        bool Complete = false;
    };

    std::unique_ptr<ICursor> Cursor_;
    std::deque<TResultSet> RemainingResultSets_;
    std::optional<std::vector<TColumnMeta>> PreparedColumnMeta_;
    std::string PreparedQuery_;
    bool IsPrepared_ = false;
    SQLSMALLINT ParamCount_ = 0;

    SQLLEN RowCount_ = -1;
    TAttributes Attributes_;
    std::string CursorName_;
    TDescriptor AppRowDesc_;
    TDescriptor AppParamDesc_;
    TDescriptor ImpRowDesc_;
    TDescriptor ImpParamDesc_;
    std::shared_ptr<TDescriptor> AppRowOwner_;
    std::shared_ptr<TDescriptor> AppParamOwner_;
    TDescriptorState RowBindings_;
    TDescriptorState ParamBindings_;
    TDescriptorState ImpParamBindings_;
    TDescriptorState ImpRowBindings_;
    uint64_t RowGeneration_ = 0;
    uint64_t ParamGeneration_ = 0;
    uint64_t ImpParamGeneration_ = 0;
    uint64_t ImpRowGeneration_ = 0;
    std::atomic<EExecutionState> ExecutionState_ = EExecutionState::Idle;
    SQLUSMALLINT NeedDataParam_ = 0;
    bool InAtExec_ = false;
    bool NeedDataTokenDelivered_ = false;
    std::vector<TAtExecValue> AtExecValues_; // indexed by parameter number
    std::vector<SQLLEN> GetDataOffsets_;

    SQLRETURN BuildParams(NYdb::TParams& out, SQLULEN paramSet);
    SQLRETURN ExecuteParamSet(SQLULEN paramSet, std::optional<SQLLEN>& affectedRows);
    SQLRETURN ExecuteInternal();
    SQLRETURN FillBoundColumns(SQLULEN row);
    std::vector<TBoundParam> GetBoundParams(SQLULEN paramSet) const;
    void EnsurePreparedColumnMeta();
    void InvalidatePreparedColumnMeta();
    void SetImpRowDesc(const std::vector<TColumnMeta>& columns);
    TDescriptor& GetAppRowDesc() { return AppRowOwner_ ? *AppRowOwner_ : AppRowDesc_; }
    TDescriptor& GetAppParamDesc() { return AppParamOwner_ ? *AppParamOwner_ : AppParamDesc_; }
    void RefreshRowBindings();
    void RefreshParamBindings();
    void StartExecution();
    void FinishExecution() noexcept;
    void CheckCancellation() const;
    std::optional<SQLRETURN> CancelExecuting();
    void CheckExecutionStatus(const TStatus& status);
    void SetCursor(std::unique_ptr<ICursor> cursor);
    void ClearResults();
    void SetResults(const NQuery::TExecuteQueryResult& result);

    void ResetForMetadata();
    struct TDescriptorAttribute {
        TDescriptor& Descriptor;
        SQLSMALLINT Field;
    };
    std::optional<TDescriptorAttribute> ResolveDescriptorAttribute(SQLINTEGER attr);

    SQLUSMALLINT FindNextNeedDataParam() const;
    std::string GetMetadataTableName(const std::string& path) const;
    bool MetadataNamespaceMatches(const TMetadataArgument& catalog,
                                  const TMetadataArgument& schema,
                                  bool catalogPatternsAllowed,
                                  bool schemaPatternsAllowed) const;

    NQuery::TExecuteQueryResult ExecuteQuery(
        NQuery::TSession& session,
        const NYdb::TParams& params,
        SQLULEN paramSet);

    NYdb::NRetry::TRetryOperationSettings MakeAutocommitRetrySettings();
    std::vector<NScheme::TSchemeEntry> GetMetadataEntries(const TMetadataArgument& tableName,
                                                          bool patternsAllowed);
    void VisitEntry(const std::string& path, const std::string& tableName,
                    bool patternsAllowed, std::string_view literalPrefix, bool hasWildcard,
                    std::vector<NScheme::TSchemeEntry>& resultEntries);
    std::optional<std::string> GetTableType(NScheme::ESchemeEntryType type);
};

} // namespace NYdb::NOdbc
