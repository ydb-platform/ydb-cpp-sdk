#include "statement.h"

#include "utils/convert.h"
#include "utils/types.h"
#include "utils/error_manager.h"
#include "utils/escape.h"
#include "utils/sql_like.h"

#include <ydb-cpp-sdk/client/params/params.h>
#include <ydb-cpp-sdk/client/value/value.h>
#include <ydb-cpp-sdk/client/retry/retry.h>
#include <ydb-cpp-sdk/library/issue/yql_issue.h>

#include <util/datetime/base.h>

#include <optional>
#include <cctype>
#include <cstring>

namespace NYdb {
namespace NOdbc {

namespace {
    NYdb::TStatus StatusFrom(const NYdb::TStatus& ydb_status) {
        return NYdb::TStatus(ydb_status.GetStatus(), NYdb::NIssue::TIssues(ydb_status.GetIssues()));
    }

    NYdb::TStatus PrefetchFirstPartStatus(NQuery::TExecuteQueryIterator& iterator, std::optional<NQuery::TExecuteQueryPart>* prefetchedResultPart){
        prefetchedResultPart->reset();
        while (true) {
            auto part = iterator.ReadNext().ExtractValueSync();
            if (part.EOS()) {
                break;
            }
            if (!part.IsSuccess()) {
                return StatusFrom(part);
    
            }
            if (part.HasResultSet()) {
                prefetchedResultPart->emplace(std::move(part));
                return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
            }
        }
        return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
    }
}

TStatement::TStatement(TConnection* conn)
    : Conn_(conn) {}

SQLRETURN TStatement::Prepare(const std::string& statementText) {
    StreamFetchError_ = false;
    RowsFetched_ = 0;
    Cursor_.reset();
    PreparedQuery_ = statementText;
    IsPrepared_ = true;
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Execute() {
    if (!IsPrepared_ || PreparedQuery_.empty()) {
        throw TOdbcException("HY007", 0, "No prepared statement");
    }
    StreamFetchError_ = false;
    RowsFetched_ = 0;
    Cursor_.reset();
    auto* client = Conn_->GetClient();
    if (!client) {
        throw TOdbcException("HY000", 0, "No client connection");
    }
    NYdb::TParams params = BuildParams();

    std::optional<NQuery::TExecuteQueryIterator> iterator;
    std::optional<NQuery::TExecuteQueryPart> prefetchedResultPart;

    if (Conn_->GetAutocommit()){
        Conn_->ResetTx();
        Conn_->ResetQuerySession();
        const NYdb::NRetry::TRetryOperationSettings retrySettings =
            MakeAutocommitRetrySettings();

        NYdb::TStatus execStatus = client->RetryQuerySync(
            [this, &params, &iterator, &prefetchedResultPart](NQuery::TSession session) -> NYdb::TStatus{
                auto retry_iterator = CreateExecuteIterator(session, params);
                if (!retry_iterator.IsSuccess()) {
                    return StatusFrom(retry_iterator);
                }
                std::optional<NQuery::TExecuteQueryPart> retry_prefetched;
                const NYdb::TStatus prefetchStatus = PrefetchFirstPartStatus(retry_iterator, &retry_prefetched);
                if (!prefetchStatus.IsSuccess()) {
                    return prefetchStatus;
                }
                iterator.emplace(std::move(retry_iterator));
                prefetchedResultPart = std::move(retry_prefetched);
                return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
            }, retrySettings);

        NStatusHelpers::ThrowOnError(execStatus);
    } else {
        NQuery::TSession& session = Conn_->GetOrCreateQuerySession();
        iterator.emplace(CreateExecuteIterator(session, params));
        NStatusHelpers::ThrowOnError(*iterator);
        NStatusHelpers::ThrowOnError(PrefetchFirstPartStatus(*iterator, &prefetchedResultPart));
    }

    if (prefetchedResultPart) {
        Cursor_ = CreateExecCursor(this, std::move(*iterator), std::move(prefetchedResultPart));
    } else {
        Cursor_.reset();
    }
    IsPrepared_ = false;
    PreparedQuery_.clear();
    return SQL_SUCCESS;
}

NYdb::NRetry::TRetryOperationSettings TStatement::MakeAutocommitRetrySettings() {
    NYdb::NRetry::TRetryOperationSettings settings;
    SQLUINTEGER queryTimeoutSec = Attributes_.GetQueryTimeoutSec();
    if (queryTimeoutSec > 0) {
        const TDuration deadline = TDuration::Seconds(queryTimeoutSec);
        settings.MaxTimeout(deadline).GetSessionClientTimeout(deadline);
    }
    return settings;
}

NQuery::TExecuteQueryIterator TStatement::CreateExecuteIterator(NQuery::TSession& session, const NYdb::TParams& params){
    const std::string sqlText = Attributes_.GetNoScanMode() == SQL_NOSCAN_ON
        ? PreparedQuery_
        : RewriteOdbcEscapes(PreparedQuery_);
    const std::string queryText = Conn_->WrapQueryForCurrentCatalog(sqlText);
    NQuery::TExecuteQuerySettings execSettings;
    const SQLUINTEGER queryTimeoutSec = Attributes_.GetQueryTimeoutSec();
    execSettings.ClientTimeout(TDuration::Seconds(queryTimeoutSec));
    const auto txSettings = Conn_->MakeTxSettings();
    if (Conn_->GetAutocommit()) {
        // TS_SNAPSHOT_RW doesn't support explicit BeginTx() - we use NoTx() instead
        // DDL must use NoTx() per YDB documentation
        const bool isSnapshotRw = (txSettings.GetMode() == NQuery::TTxSettings::TS_SNAPSHOT_RW);
        
        const bool isDdl = [&queryText] {
            size_t pos = 0;
            while (pos < queryText.size() && std::isspace(static_cast<unsigned char>(queryText[pos]))) {
                ++pos;
            }
            if (queryText.size() - pos >= 6) {
                const char* start = queryText.c_str() + pos;
                return (strncasecmp(start, "CREATE", 6) == 0 ||
                        strncasecmp(start, "DROP", 4) == 0 ||
                        strncasecmp(start, "ALTER", 5) == 0);
            }
            return false;
        }();

        if (isSnapshotRw || isDdl) {
            return session.StreamExecuteQuery(
                queryText,
                NQuery::TTxControl::NoTx(),
                params,
                execSettings).ExtractValueSync();
        }
        return session.StreamExecuteQuery(
            queryText,
            NQuery::TTxControl::BeginTx(txSettings).CommitTx(),
            params,
            execSettings).ExtractValueSync();
    }
    if (!Conn_->GetTx()) {
        auto beginTxResult = session.BeginTransaction(txSettings).ExtractValueSync();
        NStatusHelpers::ThrowOnError(beginTxResult);
        Conn_->SetTx(beginTxResult.GetTransaction());
    }
    return session.StreamExecuteQuery(
        queryText,
        NQuery::TTxControl::Tx(*Conn_->GetTx()).CommitTx(false),
        params,
        execSettings).ExtractValueSync();
}



SQLRETURN TStatement::Fetch() {
    if (!Cursor_) {
        Cursor_.reset();
        return SQL_NO_DATA;
    }
    const SQLULEN maxRows = Attributes_.GetMaxRows();
    if (maxRows > 0 && RowsFetched_ >= maxRows) {
        return SQL_NO_DATA;
    }
    StreamFetchError_ = false;
    if (!Cursor_->Fetch()) {
        return StreamFetchError_ ? SQL_ERROR : SQL_NO_DATA;
    }
    ++RowsFetched_;
    return SQL_SUCCESS;
}

void TStatement::OnStreamPartError(const TStatus& status) {
    ClearErrors();
    AddError(status);
    StreamFetchError_ = true;
}

SQLRETURN TStatement::GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, 
                              SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) {
    if (!Cursor_) {
        return SQL_NO_DATA;
    }
    return Cursor_->GetData(columnNumber, targetType, targetValue, bufferLength, strLenOrInd);
}

void TStatement::FillBoundColumns() {
    if (!Cursor_) {
        return;
    }
    for (const auto& col : BoundColumns_) {
        Cursor_->GetData(col.ColumnNumber, col.TargetType, col.TargetValue, col.BufferLength, col.StrLenOrInd);
    }
}

SQLRETURN TStatement::BindCol(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) {
    if (!Cursor_) {
        return SQL_NO_DATA;
    }

    BoundColumns_.erase(std::remove_if(BoundColumns_.begin(), BoundColumns_.end(),
            [columnNumber](const TBoundColumn& col) { return col.ColumnNumber == columnNumber; }), BoundColumns_.end());

    if (!targetValue) {
        return SQL_SUCCESS;
    }
    BoundColumns_.push_back({columnNumber, targetType, targetValue, bufferLength, strLenOrInd});
    return SQL_SUCCESS;
}

SQLRETURN TStatement::BindParameter(SQLUSMALLINT paramNumber,
                                    SQLSMALLINT inputOutputType,
                                    SQLSMALLINT valueType,
                                    SQLSMALLINT parameterType,
                                    SQLULEN columnSize,
                                    SQLSMALLINT decimalDigits,
                                    SQLPOINTER parameterValuePtr,
                                    SQLLEN bufferLength,
                                    SQLLEN* strLenOrIndPtr) {

    if (inputOutputType != SQL_PARAM_INPUT) {
        throw TOdbcException("HYC00", 0, "Only input parameters are supported");
    }

    BoundParams_.erase(std::remove_if(BoundParams_.begin(), BoundParams_.end(),
        [paramNumber](const TBoundParam& p) { return p.ParamNumber == paramNumber; }), BoundParams_.end());

    if (!parameterValuePtr) {
        return SQL_SUCCESS;
    }
    BoundParams_.push_back({paramNumber, inputOutputType, valueType, parameterType, columnSize, decimalDigits, parameterValuePtr, bufferLength, strLenOrIndPtr});
    return SQL_SUCCESS;
}

NYdb::TParams TStatement::BuildParams() {
    ClearErrors();
    NYdb::TParamsBuilder paramsBuilder;
    for (const auto& param : BoundParams_) {
        std::string paramName = "$p" + std::to_string(param.ParamNumber);
        ConvertParam(param, paramsBuilder.AddParam(paramName));
    }

    return paramsBuilder.Build();
}

SQLRETURN TStatement::Columns(const std::string& catalogName,
                              const std::string& schemaName,
                              const std::string& tableName,
                              const std::string& columnName) {
    ClearErrors();
    RowsFetched_ = 0;
    Cursor_.reset();

    std::vector<TColumnMeta> columns = {
        {"TABLE_CAT", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_SCHEM", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"COLUMN_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"DATA_TYPE", SQL_INTEGER, 0, SQL_NO_NULLS},
        {"TYPE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"COLUMN_SIZE", SQL_INTEGER, 0, SQL_NULLABLE},
        {"BUFFER_LENGTH", SQL_INTEGER, 0, SQL_NULLABLE},
        {"DECIMAL_DIGITS", SQL_INTEGER, 0, SQL_NULLABLE},
        {"NUM_PREC_RADIX", SQL_INTEGER, 0, SQL_NULLABLE},
        {"NULLABLE", SQL_INTEGER, 0, SQL_NO_NULLS},
        {"REMARKS", SQL_VARCHAR, 762, SQL_NULLABLE},
        {"COLUMN_DEF", SQL_VARCHAR, 254, SQL_NULLABLE},
        {"SQL_DATA_TYPE", SQL_INTEGER, 0, SQL_NO_NULLS},
        {"SQL_DATETIME_SUB", SQL_INTEGER, 0, SQL_NULLABLE},
        {"CHAR_OCTET_LENGTH", SQL_INTEGER, 0, SQL_NULLABLE},
        {"ORDINAL_POSITION", SQL_INTEGER, 0, SQL_NO_NULLS},
        {"IS_NULLABLE", SQL_VARCHAR, 254, SQL_NO_NULLS}
    };

    auto entries = GetPatternEntries(tableName);
    if (entries.empty()) {
        throw TOdbcException("HYC00", 0, "No tables found");
    }

    TTable table;
    table.reserve(entries.size());

    for (const auto& entry : entries) {
        if (entry.Type != NScheme::ESchemeEntryType::Table &&
            entry.Type != NScheme::ESchemeEntryType::ColumnTable) {
            continue;
        }

        auto status = Conn_->GetTableClient()->RetryOperationSync([this, path = entry.Name, &table, &columnName](NTable::TSession session) -> TStatus {
            auto result = session.DescribeTable(path).ExtractValueSync();
            NStatusHelpers::ThrowOnError(result);

            auto columns = result.GetTableDescription().GetTableColumns();

            auto columnIt = std::find_if(columns.begin(), columns.end(), [&](const NTable::TTableColumn& column) {
                if (Attributes_.GetMetadataId() == SQL_TRUE) {
                    return column.Name == columnName;
                }
                if (columnName.empty()) {
                    return column.Name.empty();
                }
                return SqlLikeMatch(column.Name, columnName);
            });

            if (columnIt == columns.end()) {
                throw TOdbcException("42S22", 0, "Column not found", SQL_ERROR);
            }

            auto column = *columnIt;

            TTypeParser typeParser(column.Type);

            table.push_back({
                TValueBuilder().OptionalUtf8(std::nullopt).Build(),
                TValueBuilder().OptionalUtf8(std::nullopt).Build(),
                TValueBuilder().Utf8(path).Build(),
                TValueBuilder().Utf8(column.Name).Build(),
                TValueBuilder().Int16(GetTypeId(column.Type)).Build(),
                TValueBuilder().Utf8(column.Type.ToString()).Build(),
                TValueBuilder().OptionalInt32(std::nullopt).Build(),
                TValueBuilder().OptionalInt32(std::nullopt).Build(),
                TValueBuilder().OptionalInt16(GetDecimalDigits(column.Type)).Build(),
                TValueBuilder().OptionalInt16(GetRadix(column.Type)).Build(),
                TValueBuilder().Int16(column.NotNull && *column.NotNull ? SQL_NO_NULLS : SQL_NULLABLE).Build(),
                TValueBuilder().OptionalUtf8(std::nullopt).Build(),
                TValueBuilder().OptionalUtf8(std::nullopt).Build(),
                TValueBuilder().Int16(GetTypeId(column.Type)).Build(),
                TValueBuilder().OptionalInt16(std::nullopt).Build(),
                TValueBuilder().OptionalInt32(8).Build(),
                TValueBuilder().OptionalInt32(columnIt - columns.begin() + 1).Build(),
                TValueBuilder().Utf8(column.NotNull && *column.NotNull ? "NO" : "YES").Build(),
            });
            return TStatus(EStatus::SUCCESS, {});
        });

        NStatusHelpers::ThrowOnError(status);
    }

    Cursor_ = CreateVirtualCursor(this, columns, table);
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Tables(const std::string& catalogName,
                             const std::string& schemaName,
                             const std::string& tableName,
                             const std::string& tableType) {
    ClearErrors();
    RowsFetched_ = 0;
    Cursor_.reset();

    std::vector<TColumnMeta> columns = {
        {"TABLE_CAT", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_SCHEM", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"TABLE_TYPE", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"REMARKS", SQL_VARCHAR, 254, SQL_NULLABLE}
    };

    auto entries = GetPatternEntries(tableName);
    if (entries.empty()) {
        throw TOdbcException("HYC00", 0, "No tables found");
    }

    TTable table;
    table.reserve(entries.size());

    for (const auto& entry : entries) {
        auto tableType = GetTableType(entry.Type);
        if (!tableType) {
            continue;
        }

        table.push_back({
            TValueBuilder().OptionalUtf8(std::nullopt).Build(),
            TValueBuilder().OptionalUtf8(std::nullopt).Build(),
            TValueBuilder().Utf8(entry.Name).Build(),
            TValueBuilder().Utf8(*tableType).Build(),
            TValueBuilder().OptionalUtf8(std::nullopt).Build(),
        });
    }

    Cursor_ = CreateVirtualCursor(this, columns, table);
    return SQL_SUCCESS;
}

std::vector<NScheme::TSchemeEntry> TStatement::GetPatternEntries(const std::string& pattern) {
    std::vector<NScheme::TSchemeEntry> entries;
    VisitEntry("", pattern, entries);
    return entries;
}

SQLRETURN TStatement::VisitEntry(const std::string& path, const std::string& pattern, std::vector<NScheme::TSchemeEntry>& resultEntries) {
    auto schemeClient = Conn_->GetSchemeClient();
    auto listDirectoryResult = schemeClient->ListDirectory(path + "/").ExtractValueSync();
    NStatusHelpers::ThrowOnError(listDirectoryResult);

    for (const auto& entry : listDirectoryResult.GetChildren()) {
        std::string fullPath = path + "/" + entry.Name;
        if (entry.Type == NScheme::ESchemeEntryType::Directory ||
            entry.Type == NScheme::ESchemeEntryType::SubDomain) {
            VisitEntry(fullPath, pattern, resultEntries);
        } else if (IsPatternMatch(fullPath, pattern)) {
            NScheme::TSchemeEntry entryCopy = entry;
            entryCopy.Name = fullPath;
            resultEntries.push_back(entryCopy);
        }
    }
    return SQL_SUCCESS;
}

bool TStatement::IsPatternMatch(const std::string& path, const std::string& pattern) {
    if (pattern.empty()) {
        return true;
    }
    if (Attributes_.GetMetadataId() == SQL_TRUE) {
        return path == pattern;
    }
    return SqlLikeMatch(path, pattern);
}

std::optional<std::string> TStatement::GetTableType(NScheme::ESchemeEntryType type) {
    switch (type) {
        case NScheme::ESchemeEntryType::Table:
            return "TABLE";
        case NScheme::ESchemeEntryType::View:
            return "VIEW";
        case NScheme::ESchemeEntryType::ColumnStore:
            return "COLUMN_STORE";
        case NScheme::ESchemeEntryType::ColumnTable:
            return "COLUMN_TABLE";
        case NScheme::ESchemeEntryType::Sequence:
            return "SEQUENCE";
        case NScheme::ESchemeEntryType::Replication:
            return "REPLICATION";
        case NScheme::ESchemeEntryType::Topic:
            return "TOPIC";
        case NScheme::ESchemeEntryType::ExternalTable:
            return "EXTERNAL_TABLE";
        case NScheme::ESchemeEntryType::ExternalDataSource:
            return "EXTERNAL_DATA_SOURCE";
        case NScheme::ESchemeEntryType::ResourcePool:
            return "RESOURCE_POOL";
        case NScheme::ESchemeEntryType::PqGroup:
            return "PQ_GROUP";
        case NScheme::ESchemeEntryType::RtmrVolume:
            return "RTMR_VOLUME";
        case NScheme::ESchemeEntryType::BlockStoreVolume:
            return "BLOCK_STORE_VOLUME";
        case NScheme::ESchemeEntryType::CoordinationNode:
            return "COORDINATION_NODE";
        case NScheme::ESchemeEntryType::Unknown:
            return "UNKNOWN";
        case NScheme::ESchemeEntryType::SysView:
            return "SYSTEM VIEW";
        case NScheme::ESchemeEntryType::Transfer:
            return "TRANSFER";
        case NScheme::ESchemeEntryType::Directory:
        case NScheme::ESchemeEntryType::SubDomain:
            return std::nullopt;
        default:
            return std::nullopt;
    }
}

SQLRETURN TStatement::Close(bool force) {
    if (!force && !Cursor_) {
        throw TOdbcException("24000", 0, "Invalid handle");
    }

    Cursor_.reset();
    RowsFetched_ = 0;
    PreparedQuery_.clear();
    IsPrepared_ = false;
    ClearErrors();
    return SQL_SUCCESS;
}

void TStatement::UnbindColumns() {
    BoundColumns_.clear();
}

void TStatement::ResetParams() {
    BoundParams_.clear();
}

SQLRETURN TStatement::RowCount(SQLLEN* rowCount) {
    if (!rowCount) {
        throw TOdbcException("HY000", 0, "Invalid parameter");
    }

    *rowCount = -1;
    return SQL_SUCCESS;
}

SQLRETURN TStatement::NumResultCols(SQLSMALLINT* colCount) {
    if (!colCount) {
        throw TOdbcException("HY000", 0, "Invalid parameter");
    }
    if (!Cursor_) {
        *colCount = 0;
        return SQL_SUCCESS;
    }
    *colCount = static_cast<SQLSMALLINT>(Cursor_->GetColumnMeta().size());
    return SQL_SUCCESS;
}

SQLRETURN TStatement::SetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER stringLength) {
    return Attributes_.SetStmtAttr(attr, value, stringLength, *this);
}

SQLRETURN TStatement::GetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr) {
    return Attributes_.GetStmtAttr(attr, value, bufferLength, stringLengthPtr, *this);
}

SQLRETURN TStatement::GetDiagField(
    SQLSMALLINT recNumber,
    SQLSMALLINT diagIdentifier,
    SQLPOINTER diagInfoPtr,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* stringLengthPtr) {
    if (recNumber == 0 && diagIdentifier == SQL_DIAG_ROW_COUNT) {
        if (!diagInfoPtr) {
            return SQL_ERROR;
        }
        *reinterpret_cast<SQLLEN*>(diagInfoPtr) = -1;
        return SQL_SUCCESS;
    }
    return TErrorManager::GetDiagField(recNumber, diagIdentifier, diagInfoPtr, bufferLength, stringLengthPtr);
}

} // namespace NOdbc
} // namespace NYdb
