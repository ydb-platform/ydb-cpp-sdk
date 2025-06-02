#include "statement.h"

#include "utils/convert.h"
#include "utils/types.h"

#include <ydb-cpp-sdk/client/params/params.h>
#include <ydb-cpp-sdk/client/value/value.h>

namespace NYdb {
namespace NOdbc {

TStatement::TStatement(TConnection* conn)
    : Conn_(conn) {}

SQLRETURN TStatement::Prepare(const std::string& statementText) {
    Cursor_.reset();
    PreparedQuery_ = statementText;
    IsPrepared_ = true;
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Execute() {
    if (!IsPrepared_ || PreparedQuery_.empty()) {
        AddError("HY007", 0, "No prepared statement");
        return SQL_ERROR;
    }
    Cursor_.reset();
    auto* client = Conn_->GetClient();
    if (!client) {
        return SQL_ERROR;
    }
    NYdb::TParams params = BuildParams();
    if (!Errors_.empty()) {
        return SQL_ERROR;
    }
    if (!Conn_->GetTx()) {
        auto sessionResult = client->GetSession().ExtractValueSync();
        if (!sessionResult.IsSuccess()) {
            return SQL_ERROR;
        }
        auto session = sessionResult.GetSession();
        auto beginTxResult = session.BeginTransaction(NQuery::TTxSettings::SerializableRW()).ExtractValueSync();
        if (!beginTxResult.IsSuccess()) {
            return SQL_ERROR;
        }
        Conn_->SetTx(beginTxResult.GetTransaction());
    }
    auto session = Conn_->GetTx()->GetSession();
    auto iterator = session.StreamExecuteQuery(PreparedQuery_,
        NQuery::TTxControl::Tx(*Conn_->GetTx()).CommitTx(Conn_->GetAutocommit()), params).ExtractValueSync();
    if (!iterator.IsSuccess()) {
        return SQL_ERROR;
    }
    Cursor_ = CreateExecCursor(this, std::move(iterator));
    IsPrepared_ = false;
    PreparedQuery_.clear();
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Fetch() {
    if (!Cursor_) {
        Cursor_.reset();
        return SQL_NO_DATA;
    }
    return Cursor_->Fetch() ? SQL_SUCCESS : SQL_NO_DATA;
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

SQLRETURN TStatement::GetDiagRec(SQLSMALLINT recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeError, 
                                SQLCHAR* messageText, SQLSMALLINT bufferLength, SQLSMALLINT* textLength) {

    if (recNumber < 1 || recNumber > (SQLSMALLINT)Errors_.size()) {
        return SQL_NO_DATA;
    }

    const auto& err = Errors_[recNumber-1];
    if (sqlState) {
        strncpy((char*)sqlState, err.SqlState.c_str(), 6);
    }

    if (nativeError) {
        *nativeError = err.NativeError;
    }

    if (messageText && bufferLength > 0) {
        strncpy((char*)messageText, err.Message.c_str(), bufferLength);
        if (textLength) {
            *textLength = (SQLSMALLINT)std::min((int)err.Message.size(), (int)bufferLength);
        }
    }
    return SQL_SUCCESS;
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
        AddError("HYC00", 0, "Only input parameters are supported");
        return SQL_ERROR;
    }

    BoundParams_.erase(std::remove_if(BoundParams_.begin(), BoundParams_.end(),
        [paramNumber](const TBoundParam& p) { return p.ParamNumber == paramNumber; }), BoundParams_.end());

    if (!parameterValuePtr) {
        return SQL_SUCCESS;
    }
    BoundParams_.push_back({paramNumber, inputOutputType, valueType, parameterType, columnSize, decimalDigits, parameterValuePtr, bufferLength, strLenOrIndPtr});
    return SQL_SUCCESS;
}

void TStatement::AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message) {
    Errors_.push_back({sqlState, nativeError, message});
}

NYdb::TParams TStatement::BuildParams() {
    Errors_.clear();
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
    Errors_.clear();
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
        AddError("HYC00", 0, "No tables found");
        return SQL_ERROR;
    }

    TTable table;
    table.reserve(entries.size());

    for (const auto& entry : entries) {
        if (entry.Type != NScheme::ESchemeEntryType::Table &&
            entry.Type != NScheme::ESchemeEntryType::ColumnTable) {
            continue;
        }

        auto status = Conn_->GetTableClient()->RetryOperationSync([path = entry.Name, &table, &columnName](NTable::TSession session) -> TStatus {
            auto result = session.DescribeTable(path).ExtractValueSync();
            if (!result.IsSuccess()) {
                return result;
            }
            auto columns = result.GetTableDescription().GetTableColumns();

            auto columnIt = std::find_if(columns.begin(), columns.end(), [&columnName](const NTable::TTableColumn& column) {
                return column.Name == columnName;
            });

            if (columnIt == columns.end()) {
                return TStatus(EStatus::NOT_FOUND, { NYdb::NIssue::TIssue("Column not found") });
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

        if (!status.IsSuccess()) {
            return SQL_ERROR;
        }
    }

    Cursor_ = CreateVirtualCursor(this, columns, table);
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Tables(const std::string& catalogName,
                             const std::string& schemaName,
                             const std::string& tableName,
                             const std::string& tableType) {
    Errors_.clear();
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
        AddError("HYC00", 0, "No tables found");
        return SQL_ERROR;
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
    if (!listDirectoryResult.IsSuccess()) {
        return SQL_ERROR;
    }
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
    return path.starts_with(pattern);
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
        case NScheme::ESchemeEntryType::Directory:
        case NScheme::ESchemeEntryType::SubDomain:
            return std::nullopt;
    }
}

SQLRETURN TStatement::Close(bool force) {
    if (!force && !Cursor_) {
        AddError("24000", 0, "Invalid handle");
        return SQL_ERROR;
    }

    Cursor_.reset();
    PreparedQuery_.clear();
    IsPrepared_ = false;
    Errors_.clear();
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
        return SQL_ERROR;
    }

    *rowCount = -1;
    return SQL_SUCCESS;
}

SQLRETURN TStatement::NumResultCols(SQLSMALLINT* colCount) {
    if (!colCount) {
        return SQL_ERROR;
    }
    if (!Cursor_) {
        *colCount = 0;
        return SQL_SUCCESS;
    }
    *colCount = static_cast<SQLSMALLINT>(Cursor_->GetColumnMeta().size());
    return SQL_SUCCESS;
}

} // namespace NOdbc
} // namespace NYdb
