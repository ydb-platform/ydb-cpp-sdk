#pragma once

#include "connection.h"
#include "descriptor.h"
#include "utils/attr.h"
#include "utils/bindings.h"
#include "utils/cursor.h"

#include <ydb-cpp-sdk/client/query/client.h>

#include <sql.h>
#include <sqlext.h>

#include <memory>
#include <optional>
#include <vector>
#include <string>


namespace NYdb::NOdbc {

class TStatement : public TErrorManager {
public:
    TStatement(TConnection* conn);
    ~TStatement();

    SQLRETURN Prepare(const std::string& statementText);
    SQLRETURN Execute();
    SQLRETURN ExecuteInternal();

    SQLRETURN Fetch();
    SQLRETURN FetchScroll(SQLSMALLINT orientation, SQLLEN offset);
    SQLRETURN GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, 
                     SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd);

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

    void DetachDescriptor(TDescriptor* desc);

    SQLRETURN RowCount(SQLLEN* rowCount);
    SQLRETURN NumResultCols(SQLSMALLINT* colCount);
    const std::vector<TColumnMeta>& GetColumnMeta() const;
    SQLRETURN SetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER stringLength);
    SQLRETURN GetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr);

    SQLRETURN GetDiagField(SQLSMALLINT recNumber, SQLSMALLINT diagIdentifier, SQLPOINTER diagInfoPtr, SQLSMALLINT bufferLength,
        SQLSMALLINT* stringLengthPtr) override;

private:
    struct TAttributes {
        SQLUINTEGER QueryTimeoutSec = 0;
        SQLULEN MaxRows = 0;
        SQLULEN NoScan = SQL_NOSCAN_OFF;
        SQLULEN MetadataId = SQL_FALSE;
        SQLULEN CursorType = SQL_CURSOR_FORWARD_ONLY;

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
        TScalarProperty<SQL_ATTR_CURSOR_TYPE, &TAttributes::CursorType>>;

    struct TAtExecValue {
        std::string Data;
        SQLLEN Indicator = 0;
        bool Complete = false;
    };

    TConnection* Conn_;
    std::unique_ptr<ICursor> Cursor_;
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
    TDescriptor* CurrentAppRowDesc_;
    TDescriptor* CurrentAppParamDesc_;
    SQLUSMALLINT NeedDataParam_ = 0;
    bool InAtExec_ = false;
    bool NeedDataTokenDelivered_ = false;
    std::vector<TAtExecValue> AtExecValues_; // indexed by parameter number
    std::vector<SQLLEN> GetDataOffsets_;

    SQLRETURN BuildParams(NYdb::TParams& out, SQLULEN paramSet);
    SQLRETURN ExecuteParamSet(SQLULEN paramSet, std::optional<SQLLEN>& affectedRows);
    SQLRETURN FillBoundColumns(SQLULEN row);
    std::vector<TBoundParam> GetBoundParams(SQLULEN paramSet) const;
    void SetCursor(std::unique_ptr<ICursor> cursor);

    void ResetForMetadata();
    struct TDescriptorAttribute {
        TDescriptor* Descriptor;
        SQLSMALLINT Field;
    };
    std::optional<TDescriptorAttribute> ResolveDescriptorAttribute(SQLINTEGER attr);

    SQLUSMALLINT FindNextNeedDataParam() const;
    std::string GetTraversalRoot(const std::string& pattern) const;
    std::string GetMetadataTableName(const std::string& path) const;
    bool MetadataNamespaceMatches(const std::string& catalog, const std::string& schema) const;

    NQuery::TExecuteQueryResult ExecuteQuery(NQuery::TSession& session, const NYdb::TParams& params);

    NYdb::NRetry::TRetryOperationSettings MakeAutocommitRetrySettings();
    std::vector<NScheme::TSchemeEntry> GetPatternEntries(const std::string& pattern);
    SQLRETURN VisitEntry(const std::string& path, const std::string& pattern, std::vector<NScheme::TSchemeEntry>& resultEntries);
    bool IsPatternMatch(const std::string& path, const std::string& pattern);
    std::optional<std::string> GetTableType(NScheme::ESchemeEntryType type);
};

} // namespace NYdb::NOdbc
