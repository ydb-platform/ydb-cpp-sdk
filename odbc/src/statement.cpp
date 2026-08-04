#include "statement.h"

#include "utils/convert.h"
#include "utils/attr.h"
#include "utils/types.h"
#include "utils/diag.h"
#include "utils/escape.h"
#include "utils/param_rewrite.h"
#include "utils/sql_like.h"
#include "utils/type_info_rows.h"
#include "utils/util.h"
#include "utils/status_util.h"

#include <ydb-cpp-sdk/client/params/params.h>
#include <ydb-cpp-sdk/client/value/value.h>
#include <ydb-cpp-sdk/client/retry/retry.h>
#include <ydb-cpp-sdk/library/issue/yql_issue.h>

#include <util/datetime/base.h>

#include <optional>
#include <cctype>
#include <algorithm>
#include <cstring>
#include <limits>

namespace NYdb::NOdbc {

namespace {

    size_t CTypeSize(SQLSMALLINT type, SQLLEN bufferLength) {
        switch (type) {
            case SQL_C_CHAR: case SQL_C_BINARY: return std::max<SQLLEN>(bufferLength, 0);
            case SQL_C_BIT: case SQL_C_TINYINT: case SQL_C_UTINYINT: return sizeof(SQLCHAR);
            case SQL_C_SHORT: case SQL_C_USHORT: return sizeof(SQLSMALLINT);
            case SQL_C_LONG: case SQL_C_ULONG: return sizeof(SQLINTEGER);
            case SQL_C_SBIGINT: case SQL_C_UBIGINT: return sizeof(SQLBIGINT);
            case SQL_C_FLOAT: return sizeof(SQLREAL);
            case SQL_C_DOUBLE: return sizeof(SQLDOUBLE);
            case SQL_C_TYPE_DATE: return sizeof(SQL_DATE_STRUCT);
            case SQL_C_TYPE_TIME: return sizeof(SQL_TIME_STRUCT);
            case SQL_C_TYPE_TIMESTAMP: return sizeof(SQL_TIMESTAMP_STRUCT);
            case SQL_C_GUID: return sizeof(SQLGUID);
            default:
                return static_cast<size_t>(std::max<SQLLEN>(bufferLength, 0));
        }
    }

    template<typename T>
    T* OffsetPointer(T* pointer, SQLULEN offset, SQLULEN row, SQLULEN stride) {
        if (!pointer) {
            return nullptr;
        }
        auto* bytes = reinterpret_cast<unsigned char*>(pointer);
        return reinterpret_cast<T*>(bytes + offset + row * stride);
    }

    TBoundParam ParamAt(const TBoundParam& param, SQLULEN row, SQLULEN bindType, SQLULEN offset) {
        TBoundParam adjusted = param;
        const SQLULEN dataStride = bindType == SQL_PARAM_BIND_BY_COLUMN
            ? CTypeSize(param.ValueType, param.BufferLength)
            : bindType;
        const SQLULEN indicatorStride = bindType == SQL_PARAM_BIND_BY_COLUMN
            ? sizeof(SQLLEN)
            : bindType;
        adjusted.ParameterValuePtr = OffsetPointer(
            static_cast<unsigned char*>(param.ParameterValuePtr), offset, row, dataStride);
        adjusted.StrLenOrIndPtr = OffsetPointer(
            param.StrLenOrIndPtr, offset, row, indicatorStride);
        return adjusted;
    }

    bool StartsWithStatement(
        std::string_view queryText,
        std::initializer_list<std::string_view> keywords) {
        size_t i = 0;
        while (i < queryText.size()) {
            if (std::isspace(static_cast<unsigned char>(queryText[i]))) {
                ++i;
            } else if (queryText[i] == '-' && i + 1 < queryText.size() && queryText[i + 1] == '-') {
                while (i < queryText.size() && queryText[i] != '\n') {
                    ++i;
                }
            } else if (queryText[i] == '/' && i + 1 < queryText.size() && queryText[i + 1] == '*') {
                i += 2;
                while (i + 1 < queryText.size() && !(queryText[i] == '*' && queryText[i + 1] == '/')) {
                    ++i;
                }
                if (i + 1 < queryText.size()) {
                    i += 2;
                } else {
                    i = queryText.size();
                }
            } else {
                break;
            }
        }
        const size_t remaining = queryText.size() - i;
        for (const std::string_view keyword : keywords) {
            if (StartsWithPrefix(
                    queryText.data() + i, remaining, keyword.data(), keyword.size())) {
                return true;
            }
        }
        return false;
    }

    std::optional<SQLLEN> ExtractAffectedRows(const NQuery::TExecuteQueryResult& result) {
        const auto& stats = result.GetStats();
        if (!stats) {
            return std::nullopt;
        }

        const uint64_t maxSqlLen = static_cast<uint64_t>(std::numeric_limits<SQLLEN>::max());
        uint64_t affectedRows = 0;
        bool hasTableAccess = false;
        for (const auto& phase : stats->GetQueryPhases()) {
            for (const auto& table : phase.GetTableAccess()) {
                hasTableAccess = true;
                const uint64_t updates = table.GetUpdates().GetRows();
                const uint64_t deletes = table.GetDeletes().GetRows();
                if (updates > maxSqlLen - affectedRows) {
                    return std::nullopt;
                }
                affectedRows += updates;
                if (deletes > maxSqlLen - affectedRows) {
                    return std::nullopt;
                }
                affectedRows += deletes;
            }
        }
        if (affectedRows == 0 && (!hasTableAccess || !result.GetResultSets().empty())) {
            return std::nullopt;
        }
        return static_cast<SQLLEN>(affectedRows);
    }

}

TStatement::TStatement(TConnection* conn)
    : Conn_(conn)
    , AppRowDesc_(EDescType::AppRow, conn)
    , AppParamDesc_(EDescType::AppParam, conn)
    , ImpRowDesc_(EDescType::ImpRow, conn)
    , ImpParamDesc_(EDescType::ImpParam, conn)
    , CurrentAppRowDesc_(&AppRowDesc_)
    , CurrentAppParamDesc_(&AppParamDesc_) {
    Conn_->RegisterStatement(this);
}

TStatement::~TStatement() {
    CurrentAppRowDesc_->Detach(this);
    CurrentAppParamDesc_->Detach(this);
    Conn_->UnregisterStatement(this);
}

void TStatement::DetachDescriptor(TDescriptor* desc) {
    if (CurrentAppRowDesc_ == desc) {
        CurrentAppRowDesc_ = &AppRowDesc_;
    }
    if (CurrentAppParamDesc_ == desc) {
        CurrentAppParamDesc_ = &AppParamDesc_;
    }
    desc->Detach(this);
}

SQLRETURN TStatement::Prepare(const std::string& statementText) {
    RowsFetched_ = 0;
    RowCount_ = -1;
    SetCursor(nullptr);
    PreparedQuery_ = statementText;
    IsPrepared_ = true;
    ParamCount_ = CountOdbcParams(PreparedQuery_);
    while (ImpParamDesc_.GetRecordCount() > ParamCount_) {
        ImpParamDesc_.RemoveRecord(ImpParamDesc_.GetRecordCount());
    }
    for (SQLSMALLINT i = 1; i <= ParamCount_; ++i) {
        TDescRecord& record = ImpParamDesc_.Record(i);
        record.Nullable = SQL_NULLABLE_UNKNOWN;
    }
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Execute() {
    if (!IsPrepared_ || PreparedQuery_.empty()) {
        throw TOdbcException("HY007", 0, "No prepared statement");
    }
    if (ParamCount_ > 0 && CurrentAppParamDesc_->GetArraySize() > 1
        && !StartsWithStatement(PreparedQuery_, {"INSERT", "UPDATE", "DELETE", "UPSERT", "REPLACE"})) {
        return AddError("HYC00", 0, "Parameter arrays are supported only for data-modification statements");
    }
    const SQLUSMALLINT next = FindNextNeedDataParam();
    if (next != 0) {
        if (CurrentAppParamDesc_->GetArraySize() > 1) {
            return AddError("HYC00", 0, "Data-at-execution parameter arrays are not supported");
        }
        NeedDataParam_ = next;
        InAtExec_ = true;
        NeedDataTokenDelivered_ = false;
        return SQL_NEED_DATA;
    }
    InAtExec_ = false;
    NeedDataParam_ = 0;
    return ExecuteInternal();
}

SQLRETURN TStatement::ExecuteInternal() {
    RowCount_ = 0;
    bool hasSuccessfulParamSet = false;
    bool rowCountUsable = true;
    const SQLULEN paramsetSize = ParamCount_ > 0 ? CurrentAppParamDesc_->GetArraySize() : 1;
    SQLUSMALLINT* const operations = CurrentAppParamDesc_->GetArrayStatusPtr();
    SQLUSMALLINT* const statuses = ImpParamDesc_.GetArrayStatusPtr();
    SQLULEN* const processed = ImpParamDesc_.GetRowsProcessedPtr();
    if (processed) {
        *processed = 0;
    }
    if (statuses) {
        std::fill_n(statuses, paramsetSize, SQL_PARAM_UNUSED);
    }

    SQLRETURN result = SQL_SUCCESS;
    for (SQLULEN paramSet = 0; paramSet < paramsetSize; ++paramSet) {
        if (operations && operations[paramSet] == SQL_PARAM_IGNORE) {
            if (processed) {
                *processed = paramSet + 1;
            }
            continue;
        }
        if (operations && operations[paramSet] != SQL_PARAM_PROCEED) {
            if (statuses) {
                statuses[paramSet] = SQL_PARAM_ERROR;
            }
            if (processed) {
                *processed = paramSet + 1;
            }
            return AddError("HY024", 0, "Invalid parameter operation value");
        }
        std::optional<SQLLEN> affectedRows;
        SQLRETURN rc;
        try {
            rc = ExecuteParamSet(paramSet, affectedRows);
        } catch (...) {
            if (!hasSuccessfulParamSet) {
                RowCount_ = -1;
            }
            throw;
        }
        if (statuses) {
            statuses[paramSet] = rc == SQL_SUCCESS_WITH_INFO
                ? SQL_PARAM_SUCCESS_WITH_INFO
                : rc == SQL_SUCCESS ? SQL_PARAM_SUCCESS : SQL_PARAM_ERROR;
        }
        if (processed) {
            *processed = paramSet + 1;
        }
        if (rc == SQL_ERROR) {
            if (!hasSuccessfulParamSet) {
                RowCount_ = -1;
            }
            return SQL_ERROR;
        }
        hasSuccessfulParamSet = true;
        if (rowCountUsable) {
            if (!affectedRows || *affectedRows > std::numeric_limits<SQLLEN>::max() - RowCount_) {
                RowCount_ = -1;
                rowCountUsable = false;
            } else {
                RowCount_ += *affectedRows;
            }
        }
        if (rc == SQL_SUCCESS_WITH_INFO) {
            result = SQL_SUCCESS_WITH_INFO;
        }
    }
    return result;
}

SQLRETURN TStatement::ExecuteParamSet(
    SQLULEN paramSet,
    std::optional<SQLLEN>& affectedRows)
{
    RowsFetched_ = 0;
    SetCursor(nullptr);
    auto client = Conn_->GetClient();
    if (!client) {
        throw TOdbcException("HY000", 0, "No client connection");
    }
    NYdb::TParams params = NYdb::TParamsBuilder().Build();
    const SQLRETURN buildRc = BuildParams(params, paramSet);
    if (buildRc != SQL_SUCCESS) {
        return buildRc;
    }

    if (Conn_->GetAutocommit()) {
        Conn_->ResetTx();
        Conn_->ResetQuerySession();
        const NYdb::NRetry::TRetryOperationSettings retrySettings = MakeAutocommitRetrySettings();

        const NYdb::TStatus execStatus = client->RetryQuerySync(
            [this, &params, &affectedRows](NQuery::TSession session) -> NYdb::TStatus {
                NQuery::TExecuteQueryResult result = ExecuteQuery(session, params);
                if (!result.IsSuccess()) {
                    return StatusFrom(result);
                }
                affectedRows = ExtractAffectedRows(result);
                SetCursor(CreateExecCursor(result));
                return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
            },
            retrySettings);

        NStatusHelpers::ThrowOnError(execStatus);
    } else {
        NQuery::TSession& session = Conn_->GetOrCreateQuerySession();
        NQuery::TExecuteQueryResult result = ExecuteQuery(session, params);
        NStatusHelpers::ThrowOnError(result);
        affectedRows = ExtractAffectedRows(result);
        SetCursor(CreateExecCursor(result));
    }
    InAtExec_ = false;
    NeedDataParam_ = 0;
    NeedDataTokenDelivered_ = false;
    for (SQLSMALLINT i = 1; i <= CurrentAppParamDesc_->GetRecordCount(); ++i) {
        if (TDescRecord* param = CurrentAppParamDesc_->FindRecord(i); param && param->AtExec) {
            param->AtExecComplete = false;
            param->AtExecChunk.clear();
        }
    }
    return SQL_SUCCESS;
}

SQLUSMALLINT TStatement::FindNextNeedDataParam() const {
    for (SQLSMALLINT i = 1; i <= CurrentAppParamDesc_->GetRecordCount(); ++i) {
        const TDescRecord* record = CurrentAppParamDesc_->FindRecord(i);
        if (record && record->AtExec && !record->AtExecComplete) {
            return static_cast<SQLUSMALLINT>(i);
        }
    }
    return 0;
}

NYdb::NRetry::TRetryOperationSettings TStatement::MakeAutocommitRetrySettings() {
    NYdb::NRetry::TRetryOperationSettings settings;
    settings.Idempotent(false);
    SQLUINTEGER queryTimeoutSec = Attributes_.GetQueryTimeoutSec();
    if (queryTimeoutSec > 0) {
        const TDuration deadline = TDuration::Seconds(queryTimeoutSec);
        settings.MaxTimeout(deadline).GetSessionClientTimeout(deadline);
    }
    return settings;
}

NQuery::TExecuteQueryResult TStatement::ExecuteQuery(
    NQuery::TSession& session,
    const NYdb::TParams& params)
{
    const std::string sqlAfterEscapes = Attributes_.GetNoScanMode() == SQL_NOSCAN_ON
        ? PreparedQuery_
        : RewriteOdbcEscapes(PreparedQuery_);
    const std::vector<TBoundParam> activeParams = GetBoundParams(0);
    const TParamRewriteResult rewritten = RewriteOdbcQuestionMarks(sqlAfterEscapes, activeParams);
    if (!rewritten.Success) {
        throw TOdbcException(rewritten.SqlState, 0, rewritten.Message);
    }
    const bool isDdl = StartsWithStatement(
        rewritten.Sql, {"CREATE", "DROP", "ALTER", "GRANT", "REVOKE"});
    const std::string queryText = Conn_->WrapQueryForCurrentCatalog(rewritten.Sql);
    NQuery::TExecuteQuerySettings execSettings;
    execSettings.StatsMode(NQuery::EStatsMode::Basic);
    const SQLUINTEGER queryTimeoutSec = Attributes_.GetQueryTimeoutSec();
    if (queryTimeoutSec > 0) {
        execSettings.ClientTimeout(TDuration::Seconds(queryTimeoutSec));
    }
    const auto txSettings = Conn_->MakeTxSettings();
    if (Conn_->GetAutocommit()) {
        // TS_SNAPSHOT_RW doesn't support explicit BeginTx() - we use NoTx() instead
        // DDL must use NoTx() per YDB documentation
        const bool isSnapshotRw = (txSettings.GetMode() == NQuery::TTxSettings::TS_SNAPSHOT_RW);

        if (isSnapshotRw || isDdl) {
            return session.ExecuteQuery(
                queryText,
                NQuery::TTxControl::NoTx(),
                params,
                execSettings).ExtractValueSync();
        }
        return session.ExecuteQuery(
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
    return session.ExecuteQuery(
        queryText,
        NQuery::TTxControl::Tx(*Conn_->GetTx()).CommitTx(false),
        params,
        execSettings).ExtractValueSync();
}



SQLRETURN TStatement::Fetch() {
    if (!Cursor_) {
        return SQL_NO_DATA;
    }
    const SQLULEN maxRows = Attributes_.GetMaxRows();
    if (maxRows > 0 && RowsFetched_ >= maxRows) {
        return SQL_NO_DATA;
    }
    const SQLULEN rowArraySize = CurrentAppRowDesc_->GetArraySize();
    SQLUSMALLINT* const statuses = ImpRowDesc_.GetArrayStatusPtr();
    SQLULEN* const fetched = ImpRowDesc_.GetRowsProcessedPtr();
    if (fetched) {
        *fetched = 0;
    }
    if (statuses) {
        std::fill_n(statuses, rowArraySize, SQL_ROW_NOROW);
    }

    SQLULEN rows = 0;
    SQLRETURN result = SQL_SUCCESS;
    for (; rows < rowArraySize; ++rows) {
        if (maxRows > 0 && RowsFetched_ >= maxRows) {
            break;
        }
        BindingRow_ = rows;
        if (!Cursor_->Fetch()) {
            break;
        }
        FillBoundColumns();
        ++RowsFetched_;
        GetDataOffsets_.assign(Cursor_->GetColumnMeta().size(), 0);
        if (fetched) {
            *fetched = rows + 1;
        }
        if (statuses) {
            statuses[rows] = LastFetchRc_ == SQL_SUCCESS_WITH_INFO
                ? SQL_ROW_SUCCESS_WITH_INFO
                : LastFetchRc_ == SQL_SUCCESS ? SQL_ROW_SUCCESS : SQL_ROW_ERROR;
        }
        if (LastFetchRc_ == SQL_ERROR) {
            result = SQL_ERROR;
        } else if (LastFetchRc_ == SQL_SUCCESS_WITH_INFO && result == SQL_SUCCESS) {
            result = SQL_SUCCESS_WITH_INFO;
        }
    }
    BindingRow_ = 0;
    return rows == 0 && result != SQL_ERROR ? SQL_NO_DATA : result;
}

SQLRETURN TStatement::GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, 
                              SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) {
    if (!Cursor_) {
        return SQL_NO_DATA;
    }
    if (columnNumber < 1 || columnNumber > GetDataOffsets_.size()) {
        return AddError("07009", 0, "Invalid descriptor index");
    }
    const SQLRETURN rc = Cursor_->GetData(
        columnNumber, targetType, targetValue, bufferLength, strLenOrInd,
        &GetDataOffsets_[columnNumber - 1]);
    if (const char* sqlState = ConsumeLastConvertSqlState()) {
        AddError(sqlState, 0, std::strcmp(sqlState, "22003") == 0 ? "Numeric value out of range" : "Conversion error");
    }
    return rc;
}

void TStatement::FillBoundColumns() {
    if (!Cursor_) {
        return;
    }
    LastFetchRc_ = SQL_SUCCESS;
    const SQLULEN bindType = CurrentAppRowDesc_->GetBindType();
    const SQLULEN offset = CurrentAppRowDesc_->GetBindOffsetPtr()
        ? *CurrentAppRowDesc_->GetBindOffsetPtr()
        : 0;
    for (SQLSMALLINT number = 1; number <= CurrentAppRowDesc_->GetRecordCount(); ++number) {
        const TDescRecord* col = CurrentAppRowDesc_->FindRecord(number);
        if (!col || !col->DataPtr) {
            continue;
        }
        const SQLULEN dataStride = bindType == SQL_BIND_BY_COLUMN
            ? CTypeSize(col->Type, col->OctetLength)
            : bindType;
        const SQLULEN indicatorStride = bindType == SQL_BIND_BY_COLUMN
            ? sizeof(SQLLEN)
            : bindType;
        SQLPOINTER target = OffsetPointer(
            static_cast<unsigned char*>(col->DataPtr), offset, BindingRow_, dataStride);
        SQLLEN* indicator = OffsetPointer(
            col->IndicatorPtr, offset, BindingRow_, indicatorStride);
        SQLLEN* length = OffsetPointer(
            col->OctetLengthPtr, offset, BindingRow_, indicatorStride);
        SQLLEN convertedLength = 0;
        SQLRETURN rc = Cursor_->GetData(
            static_cast<SQLUSMALLINT>(number), col->Type, target, col->OctetLength,
            &convertedLength);
        if (convertedLength == SQL_NULL_DATA) {
            if (!indicator) {
                AddError("22002", 0, "Indicator variable required but not supplied");
                rc = SQL_ERROR;
            } else {
                *indicator = SQL_NULL_DATA;
                if (length && length != indicator) {
                    *length = 0;
                }
            }
        } else {
            if (length) {
                *length = convertedLength;
            }
            if (indicator && indicator != length) {
                *indicator = 0;
            }
        }
        if (rc == SQL_SUCCESS_WITH_INFO) {
            AddError("01004", 0, "String data, right truncated", SQL_SUCCESS_WITH_INFO);
            if (LastFetchRc_ == SQL_SUCCESS) {
                LastFetchRc_ = SQL_SUCCESS_WITH_INFO;
            }
        } else if (rc != SQL_SUCCESS && LastFetchRc_ == SQL_SUCCESS) {
            if (const char* sqlState = ConsumeLastConvertSqlState()) {
                AddError(sqlState, 0, std::strcmp(sqlState, "22003") == 0 ? "Numeric value out of range" : "Conversion error");
            }
            LastFetchRc_ = rc;
        }
    }
}

SQLRETURN TStatement::BindCol(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) {
    if (targetValue && columnNumber < 1) {
        return AddError("07009", 0, "Invalid descriptor index");
    }
    if (Cursor_) {
        const size_t n = Cursor_->GetColumnMeta().size();
        if (targetValue && n > 0 && static_cast<size_t>(columnNumber) > n) {
            return AddError("07009", 0, "Invalid descriptor index");
        }
    }

    if (!targetValue) {
        CurrentAppRowDesc_->RemoveRecord(static_cast<SQLSMALLINT>(columnNumber));
        return SQL_SUCCESS;
    }
    TDescRecord& record = CurrentAppRowDesc_->Record(static_cast<SQLSMALLINT>(columnNumber));
    record.Type = targetType;
    record.Length = bufferLength;
    record.OctetLength = bufferLength;
    record.DataPtr = targetValue;
    record.IndicatorPtr = strLenOrInd;
    record.OctetLengthPtr = strLenOrInd;
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

    const bool atExec = strLenOrIndPtr
        && (*strLenOrIndPtr == SQL_DATA_AT_EXEC
            || *strLenOrIndPtr <= SQL_LEN_DATA_AT_EXEC_OFFSET);

    if (!parameterValuePtr && !strLenOrIndPtr) {
        CurrentAppParamDesc_->RemoveRecord(static_cast<SQLSMALLINT>(paramNumber));
        ImpParamDesc_.RemoveRecord(static_cast<SQLSMALLINT>(paramNumber));
        return SQL_SUCCESS;
    }
    TDescRecord& app = CurrentAppParamDesc_->Record(static_cast<SQLSMALLINT>(paramNumber));
    app.Type = valueType;
    app.Length = bufferLength;
    app.OctetLength = bufferLength;
    app.DataPtr = parameterValuePtr;
    app.IndicatorPtr = strLenOrIndPtr;
    app.OctetLengthPtr = strLenOrIndPtr;
    app.ParameterType = inputOutputType;
    app.AtExec = atExec;
    app.AtExecComplete = false;
    app.AtExecIndicator = 0;
    app.AtExecChunk.clear();

    TDescRecord& imp = ImpParamDesc_.Record(static_cast<SQLSMALLINT>(paramNumber));
    imp.Type = parameterType;
    imp.Length = static_cast<SQLLEN>(columnSize);
    imp.OctetLength = static_cast<SQLLEN>(columnSize);
    imp.Precision = static_cast<SQLSMALLINT>(columnSize);
    imp.Scale = decimalDigits;
    imp.Nullable = SQL_NULLABLE;
    imp.ParameterType = inputOutputType;
    return SQL_SUCCESS;
}

std::vector<TBoundParam> TStatement::GetBoundParams(SQLULEN paramSet) const {
    std::vector<TBoundParam> params;
    const SQLULEN offset = CurrentAppParamDesc_->GetBindOffsetPtr()
        ? *CurrentAppParamDesc_->GetBindOffsetPtr()
        : 0;
    for (SQLSMALLINT number = 1; number <= ParamCount_; ++number) {
        const TDescRecord* app = CurrentAppParamDesc_->FindRecord(number);
        const TDescRecord* imp = ImpParamDesc_.FindRecord(number);
        if (!app || !imp) {
            continue;
        }
        SQLLEN* lengthOrIndicator = app->IndicatorPtr == app->OctetLengthPtr
            ? app->IndicatorPtr
            : app->OctetLengthPtr;
        TBoundParam param{
            static_cast<SQLUSMALLINT>(number), imp->ParameterType, app->Type, imp->Type,
            static_cast<SQLULEN>(imp->Length), imp->Scale, app->DataPtr, app->OctetLength,
            lengthOrIndicator, app->AtExec, app->AtExecComplete, app->AtExecChunk};
        param = ParamAt(param, paramSet, CurrentAppParamDesc_->GetBindType(), offset);
        SQLLEN* indicator = OffsetPointer(
            app->IndicatorPtr, offset, paramSet,
            CurrentAppParamDesc_->GetBindType() == SQL_PARAM_BIND_BY_COLUMN
                ? sizeof(SQLLEN)
                : CurrentAppParamDesc_->GetBindType());
        if (indicator && *indicator == SQL_NULL_DATA) {
            param.StrLenOrIndPtr = indicator;
        }
        params.push_back(std::move(param));
    }
    return params;
}

SQLRETURN TStatement::BuildParams(NYdb::TParams& out, SQLULEN paramSet) {
    ClearErrors();
    NYdb::TParamsBuilder paramsBuilder;
    for (const TBoundParam& param : GetBoundParams(paramSet)) {
        const std::string paramName = "$p" + std::to_string(param.ParamNumber);
        if (param.AtExec) {
            if (!param.AtExecComplete) {
                return AddError("HY000", 0, "Missing data-at-execution parameter value");
            }
            const TDescRecord* record = CurrentAppParamDesc_->FindRecord(
                static_cast<SQLSMALLINT>(param.ParamNumber));
            SQLLEN indicator = record && record->AtExecIndicator == SQL_NULL_DATA
                ? SQL_NULL_DATA
                : SQL_NTS;
            TBoundParam tmp = param;
            tmp.ParameterValuePtr = const_cast<char*>(param.AtExecChunk.data());
            tmp.StrLenOrIndPtr = &indicator;
            const SQLRETURN convRc = ConvertParam(tmp, paramsBuilder.AddParam(paramName));
            if (convRc != SQL_SUCCESS) {
                return AddError("07006", 0, "Unsupported or invalid ODBC parameter type for parameter "
                    + std::to_string(param.ParamNumber));
            }
            continue;
        }
        const SQLRETURN convRc = ConvertParam(param, paramsBuilder.AddParam(paramName));
        if (convRc != SQL_SUCCESS) {
            return AddError(
                "07006",
                0,
                "Unsupported or invalid ODBC parameter type for parameter " + std::to_string(param.ParamNumber)
                    + " (C type " + std::to_string(static_cast<int>(param.ValueType)) + ", SQL type "
                    + std::to_string(static_cast<int>(param.ParameterType)) + ")");
        }
    }
    out = paramsBuilder.Build();
    return SQL_SUCCESS;
}


SQLRETURN TStatement::NumParams(SQLSMALLINT* paramCount) {
    if (!paramCount) {
        throw TOdbcException("HY000", 0, "Invalid parameter");
    }
    if (!IsPrepared_) {
        throw TOdbcException("HY010", 0, "Function sequence error");
    }
    *paramCount = ParamCount_;
    return SQL_SUCCESS;
}

void TStatement::ResetForMetadata() {
    ClearErrors();
    RowsFetched_ = 0;
    RowCount_ = -1;
    SetCursor(nullptr);
}

SQLRETURN TStatement::DescribeParam(SQLUSMALLINT paramNumber, SQLSMALLINT* dataTypePtr, SQLULEN* paramSizePtr,
                                    SQLSMALLINT* decimalDigitsPtr, SQLSMALLINT* nullablePtr) {
    if (!IsPrepared_) {
        throw TOdbcException("HY010", 0, "Function sequence error");
    }
    if (paramNumber < 1 || paramNumber > ParamCount_) {
        throw TOdbcException("07009", 0, "Invalid descriptor index");
    }
    const TDescRecord* record = ImpParamDesc_.FindRecord(static_cast<SQLSMALLINT>(paramNumber));
    const SQLSMALLINT dataType = record ? record->Type : SQL_UNKNOWN_TYPE;
    const SQLULEN paramSize = record ? static_cast<SQLULEN>(record->Length) : 0;
    const SQLSMALLINT decimalDigits = record ? record->Scale : 0;
    const SQLSMALLINT nullable = record ? record->Nullable : SQL_NULLABLE_UNKNOWN;
    if (dataTypePtr) {
        *dataTypePtr = dataType;
    }
    if (paramSizePtr) {
        *paramSizePtr = paramSize;
    }
    if (decimalDigitsPtr) {
        *decimalDigitsPtr = decimalDigits;
    }
    if (nullablePtr) {
        *nullablePtr = nullable;
    }
    return SQL_SUCCESS;
}

SQLRETURN TStatement::ParamData(SQLPOINTER* valuePtr) {
    if (!valuePtr) {
        throw TOdbcException("HY009", 0, "Invalid use of null pointer");
    }
    if (!InAtExec_) {
        return SQL_NO_DATA;
    }
    if (NeedDataParam_ != 0 && NeedDataTokenDelivered_) {
        if (TDescRecord* record = CurrentAppParamDesc_->FindRecord(
                static_cast<SQLSMALLINT>(NeedDataParam_))) {
            record->AtExecComplete = true;
        }
        NeedDataParam_ = 0;
        NeedDataTokenDelivered_ = false;
    }
    const SQLUSMALLINT next = FindNextNeedDataParam();
    if (next != 0) {
        NeedDataParam_ = next;
        NeedDataTokenDelivered_ = true;
        *valuePtr = CurrentAppParamDesc_->FindRecord(static_cast<SQLSMALLINT>(next))->DataPtr;
        return SQL_NEED_DATA;
    }
    InAtExec_ = false;
    NeedDataParam_ = 0;
    return ExecuteInternal();
}

SQLRETURN TStatement::PutData(SQLPOINTER data, SQLLEN strLenOrInd) {
    if (!InAtExec_ || NeedDataParam_ == 0) {
        throw TOdbcException("HY010", 0, "Function sequence error");
    }
    TDescRecord* param = CurrentAppParamDesc_->FindRecord(
        static_cast<SQLSMALLINT>(NeedDataParam_));
    if (!param || !NeedDataTokenDelivered_) {
        throw TOdbcException("HY010", 0, "Function sequence error");
    }
    SQLLEN chunkLen = strLenOrInd;
    if (chunkLen == SQL_NULL_DATA) {
        param->AtExecIndicator = SQL_NULL_DATA;
        return SQL_SUCCESS;
    }
    if (chunkLen == SQL_DEFAULT_PARAM) {
        throw TOdbcException("07S01", 0, "Default parameters are not supported");
    }
    if (chunkLen == SQL_NTS) {
        if (!data) {
            throw TOdbcException("HY009", 0, "Invalid use of null pointer");
        }
        chunkLen = static_cast<SQLLEN>(std::strlen(static_cast<const char*>(data)));
    }
    if (chunkLen < 0) {
        throw TOdbcException("HY090", 0, "Invalid string or buffer length");
    }
    if (chunkLen > 0) {
        if (!data) {
            throw TOdbcException("HY009", 0, "Invalid use of null pointer");
        }
        param->AtExecChunk.append(static_cast<const char*>(data), static_cast<size_t>(chunkLen));
    }
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Cancel() {
    if (!Cursor_ && !InAtExec_) {
        return SQL_SUCCESS;
    }
    SetCursor(nullptr);
    InAtExec_ = false;
    NeedDataParam_ = 0;
    NeedDataTokenDelivered_ = false;
    for (SQLSMALLINT i = 1; i <= CurrentAppParamDesc_->GetRecordCount(); ++i) {
        if (TDescRecord* param = CurrentAppParamDesc_->FindRecord(i)) {
            param->AtExecComplete = false;
            param->AtExecIndicator = 0;
            param->AtExecChunk.clear();
        }
    }
    RowsFetched_ = 0;
    return SQL_SUCCESS;
}

SQLRETURN TStatement::SetCursorName(const std::string& name) {
    CursorName_ = name;
    return SQL_SUCCESS;
}

SQLRETURN TStatement::GetCursorName(SQLCHAR* name, SQLSMALLINT bufferLength, SQLSMALLINT* nameLengthPtr) {
    return Diag::WriteOdbcString(*this, CursorName_, name, bufferLength, nameLengthPtr);
}


SQLRETURN TStatement::Close(bool force) {
    if (!force && !Cursor_) {
        throw TOdbcException("24000", 0, "Invalid handle");
    }

    SetCursor(nullptr);
    RowsFetched_ = 0;
    ClearErrors();
    return SQL_SUCCESS;
}

void TStatement::UnbindColumns() {
    CurrentAppRowDesc_->ClearRecords();
}

void TStatement::ResetParams() {
    CurrentAppParamDesc_->ClearRecords();
    ImpParamDesc_.ClearRecords();
}

SQLRETURN TStatement::RowCount(SQLLEN* rowCount) {
    if (!rowCount) {
        throw TOdbcException("HY000", 0, "Invalid parameter");
    }

    *rowCount = RowCount_;
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

const std::vector<TColumnMeta>& TStatement::GetColumnMeta() const {
    static const std::vector<TColumnMeta> EmptyColumns;
    return Cursor_ ? Cursor_->GetColumnMeta() : EmptyColumns;
}

void TStatement::SetCursor(std::unique_ptr<ICursor> cursor) {
    Cursor_ = std::move(cursor);
    GetDataOffsets_.clear();
    ImpRowDesc_.ClearRecords();
    if (!Cursor_) {
        return;
    }
    SQLSMALLINT number = 0;
    for (const TColumnMeta& column : Cursor_->GetColumnMeta()) {
        TDescRecord& record = ImpRowDesc_.Record(++number);
        record.Name = column.Name;
        record.Type = column.SqlType;
        record.Length = static_cast<SQLLEN>(column.Size);
        record.OctetLength = static_cast<SQLLEN>(column.Size);
        record.Precision = static_cast<SQLSMALLINT>(column.Size);
        record.Scale = column.DecimalDigits;
        record.Nullable = column.Nullable;
    }
}

SQLRETURN TStatement::SetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER stringLength) {
    if (attr == SQL_ATTR_APP_ROW_DESC || attr == SQL_ATTR_APP_PARAM_DESC) {
        TDescriptor* desc = value ? TDescriptor::FromHandle(value) : nullptr;
        if (desc && (desc->GetDescType() != EDescType::Explicit
                     || desc->GetConnection() != Conn_)) {
            return AddError("HY024", 0, "Descriptor belongs to another connection");
        }
        TDescriptor*& current = attr == SQL_ATTR_APP_ROW_DESC
            ? CurrentAppRowDesc_
            : CurrentAppParamDesc_;
        TDescriptor* const automatic = attr == SQL_ATTR_APP_ROW_DESC
            ? &AppRowDesc_
            : &AppParamDesc_;
        TDescriptor* const next = desc ? desc : automatic;
        if (current != next) {
            TDescriptor* const previous = current;
            current = next;
            current->Attach(this);
            if (CurrentAppRowDesc_ != previous && CurrentAppParamDesc_ != previous) {
                previous->Detach(this);
            }
        }
        return SQL_SUCCESS;
    }
    const SQLULEN integer = ReadIntegerAttr<SQLULEN>(value);
    switch (attr) {
        case SQL_ATTR_PARAM_BIND_TYPE: CurrentAppParamDesc_->SetBindType(integer); return SQL_SUCCESS;
        case SQL_ATTR_PARAMSET_SIZE:
            if (integer == 0) return Diag::AddInvalidAttrValue(*this, "SQL_ATTR_PARAMSET_SIZE");
            CurrentAppParamDesc_->SetArraySize(integer); return SQL_SUCCESS;
        case SQL_ATTR_PARAM_BIND_OFFSET_PTR:
            CurrentAppParamDesc_->SetBindOffsetPtr(static_cast<SQLULEN*>(value)); return SQL_SUCCESS;
        case SQL_ATTR_PARAM_OPERATION_PTR:
            CurrentAppParamDesc_->SetArrayStatusPtr(static_cast<SQLUSMALLINT*>(value)); return SQL_SUCCESS;
        case SQL_ATTR_PARAM_STATUS_PTR:
            ImpParamDesc_.SetArrayStatusPtr(static_cast<SQLUSMALLINT*>(value)); return SQL_SUCCESS;
        case SQL_ATTR_PARAMS_PROCESSED_PTR:
            ImpParamDesc_.SetRowsProcessedPtr(static_cast<SQLULEN*>(value)); return SQL_SUCCESS;
        case SQL_ATTR_ROW_BIND_TYPE: CurrentAppRowDesc_->SetBindType(integer); return SQL_SUCCESS;
        case SQL_ATTR_ROW_ARRAY_SIZE:
            if (integer == 0) return Diag::AddInvalidAttrValue(*this, "SQL_ATTR_ROW_ARRAY_SIZE");
            CurrentAppRowDesc_->SetArraySize(integer); return SQL_SUCCESS;
        case SQL_ATTR_ROW_BIND_OFFSET_PTR:
            CurrentAppRowDesc_->SetBindOffsetPtr(static_cast<SQLULEN*>(value)); return SQL_SUCCESS;
        case SQL_ATTR_ROW_STATUS_PTR:
            ImpRowDesc_.SetArrayStatusPtr(static_cast<SQLUSMALLINT*>(value)); return SQL_SUCCESS;
        case SQL_ATTR_ROWS_FETCHED_PTR:
            ImpRowDesc_.SetRowsProcessedPtr(static_cast<SQLULEN*>(value)); return SQL_SUCCESS;
        default: break;
    }
    return Attributes_.SetStmtAttr(attr, value, stringLength, *this);
}

SQLRETURN TStatement::GetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr) {
    if (!value) {
        return AddError("HY009", 0, "Invalid use of null pointer");
    }
    switch (attr) {
        case SQL_ATTR_APP_ROW_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = CurrentAppRowDesc_;
            return SQL_SUCCESS;
        case SQL_ATTR_APP_PARAM_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = CurrentAppParamDesc_;
            return SQL_SUCCESS;
        case SQL_ATTR_IMP_ROW_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = &ImpRowDesc_;
            return SQL_SUCCESS;
        case SQL_ATTR_IMP_PARAM_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = &ImpParamDesc_;
            return SQL_SUCCESS;
        case SQL_ATTR_PARAM_BIND_TYPE:
            *static_cast<SQLULEN*>(value) = CurrentAppParamDesc_->GetBindType(); return SQL_SUCCESS;
        case SQL_ATTR_PARAMSET_SIZE:
            *static_cast<SQLULEN*>(value) = CurrentAppParamDesc_->GetArraySize(); return SQL_SUCCESS;
        case SQL_ATTR_PARAM_BIND_OFFSET_PTR:
            *static_cast<SQLPOINTER*>(value) = CurrentAppParamDesc_->GetBindOffsetPtr(); return SQL_SUCCESS;
        case SQL_ATTR_PARAM_OPERATION_PTR:
            *static_cast<SQLPOINTER*>(value) = CurrentAppParamDesc_->GetArrayStatusPtr(); return SQL_SUCCESS;
        case SQL_ATTR_PARAM_STATUS_PTR:
            *static_cast<SQLPOINTER*>(value) = ImpParamDesc_.GetArrayStatusPtr(); return SQL_SUCCESS;
        case SQL_ATTR_PARAMS_PROCESSED_PTR:
            *static_cast<SQLPOINTER*>(value) = ImpParamDesc_.GetRowsProcessedPtr(); return SQL_SUCCESS;
        case SQL_ATTR_ROW_BIND_TYPE:
            *static_cast<SQLULEN*>(value) = CurrentAppRowDesc_->GetBindType(); return SQL_SUCCESS;
        case SQL_ATTR_ROW_ARRAY_SIZE:
            *static_cast<SQLULEN*>(value) = CurrentAppRowDesc_->GetArraySize(); return SQL_SUCCESS;
        case SQL_ATTR_ROW_BIND_OFFSET_PTR:
            *static_cast<SQLPOINTER*>(value) = CurrentAppRowDesc_->GetBindOffsetPtr(); return SQL_SUCCESS;
        case SQL_ATTR_ROW_STATUS_PTR:
            *static_cast<SQLPOINTER*>(value) = ImpRowDesc_.GetArrayStatusPtr(); return SQL_SUCCESS;
        case SQL_ATTR_ROWS_FETCHED_PTR:
            *static_cast<SQLPOINTER*>(value) = ImpRowDesc_.GetRowsProcessedPtr(); return SQL_SUCCESS;
        default:
            break;
    }
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

} // namespace NYdb::NOdbc
