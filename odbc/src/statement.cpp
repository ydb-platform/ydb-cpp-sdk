#include "statement.h"

#include "utils/convert.h"
#include "utils/attr.h"
#include "utils/diag.h"
#include "utils/param_rewrite.h"
#include "utils/status_util.h"

#include <ydb-cpp-sdk/client/params/params.h>
#include <ydb-cpp-sdk/client/value/value.h>
#include <ydb-cpp-sdk/client/retry/retry.h>
#include <ydb-cpp-sdk/library/issue/yql_issue.h>

#include <util/datetime/base.h>
#include <util/generic/scope.h>

#include <optional>
#include <algorithm>
#include <cstring>
#include <limits>
#include <string_view>

namespace NYdb::NOdbc {

namespace {

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

    std::string BuildResultMetadataQuery(std::string_view query) {
        query = TrimTrailingSqlTrivia(query);
        std::string_view statement = GetSqlStatement(query);
        const std::string_view prologue = query.substr(0, query.size() - statement.size());
        if (!statement.empty() && statement.back() == ';') {
            statement.remove_suffix(1);
            statement = TrimTrailingSqlTrivia(statement);
        }
        return std::string(prologue) + "SELECT * FROM (\n" + std::string(statement)
            + "\n) AS __ydb_odbc_result_metadata LIMIT 0";
    }

}

TStatement::TStatement(TConnection* conn)
    : Conn_(conn)
    , AppRowDesc_(EDescType::AppRow, conn)
    , AppParamDesc_(EDescType::AppParam, conn)
    , ImpRowDesc_(EDescType::ImpRow, conn)
    , ImpParamDesc_(EDescType::ImpParam, conn) {
    SetLifecycle(conn->GetLifecycle());
}

void TStatement::RegisterDescriptors(const std::shared_ptr<TStatement>& owner) {
    for (auto* desc : {&AppRowDesc_, &AppParamDesc_, &ImpRowDesc_, &ImpParamDesc_}) {
        RegisterHandle(desc, std::shared_ptr<THandle>(owner, desc));
    }
}

void TStatement::UnregisterDescriptors() {
    for (auto* desc : {&AppRowDesc_, &AppParamDesc_, &ImpRowDesc_, &ImpParamDesc_}) {
        desc->Retire();
        UnregisterHandle(desc);
    }
}

void TStatement::BeforeCall() {
    if (AppRowOwner_ && AppRowOwner_->IsRetired()) {
        AppRowOwner_.reset();
        RowGeneration_ = 0;
    }
    if (AppParamOwner_ && AppParamOwner_->IsRetired()) {
        AppParamOwner_.reset();
        ParamGeneration_ = 0;
        AtExecValues_.clear();
        InvalidatePreparedColumnMeta();
    }
}

void TStatement::RefreshRowBindings() {
    if (GetAppRowDesc().GetGeneration() != RowGeneration_) {
        GetAppRowDesc().Snapshot(RowBindings_, RowGeneration_);
    }
    if (ImpRowDesc_.GetGeneration() != ImpRowGeneration_) {
        ImpRowDesc_.SnapshotHeader(ImpRowBindings_, ImpRowGeneration_);
    }
}

void TStatement::RefreshParamBindings() {
    const auto paramSchema = ParamBindings_.GetSchemaGeneration();
    const auto impSchema = ImpParamBindings_.GetSchemaGeneration();
    if (GetAppParamDesc().GetGeneration() != ParamGeneration_) {
        GetAppParamDesc().Snapshot(ParamBindings_, ParamGeneration_);
    }
    if (ImpParamDesc_.GetGeneration() != ImpParamGeneration_) {
        ImpParamDesc_.Snapshot(ImpParamBindings_, ImpParamGeneration_);
    }
    if (paramSchema != ParamBindings_.GetSchemaGeneration()
        || impSchema != ImpParamBindings_.GetSchemaGeneration()) {
        InvalidatePreparedColumnMeta();
    }
}

SQLRETURN TStatement::Prepare(const std::string& statementText) {
    RowCount_ = -1;
    PreparedColumnMeta_.reset();
    ClearResults();
    PreparedQuery_ = statementText;
    IsPrepared_ = true;
    ParamCount_ = CountOdbcParams(PreparedQuery_);
    ImpParamDesc_.WithLock([&](TDescriptor& desc) {
        while (desc.GetRecordCount() > ParamCount_) {
            desc.RemoveRecord(desc.GetRecordCount());
        }
        for (SQLSMALLINT i = 1; i <= ParamCount_; ++i) {
            desc.Record(i).Nullable = SQL_NULLABLE_UNKNOWN;
        }
    });
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Execute() {
    if (!IsPrepared_ || PreparedQuery_.empty()) {
        throw TOdbcException("HY007", 0, "No prepared statement");
    }
    RefreshParamBindings();
    ClearResults();
    RowCount_ = -1;
    if (ParamCount_ > 0 && ParamBindings_.GetArraySize() > 1
        && !StartsWithSqlStatement(PreparedQuery_, {"INSERT", "UPDATE", "DELETE", "UPSERT", "REPLACE"})) {
        return AddError("HYC00", 0, "Parameter arrays are supported only for data-modification statements");
    }
    const SQLUSMALLINT next = FindNextNeedDataParam();
    if (next != 0) {
        if (ParamBindings_.GetArraySize() > 1) {
            return AddError("HYC00", 0, "Data-at-execution parameter arrays are not supported");
        }
        NeedDataParam_ = next;
        InAtExec_ = true;
        NeedDataTokenDelivered_ = false;
        return SQL_NEED_DATA;
    }
    InAtExec_ = false;
    NeedDataParam_ = 0;
    return Conn_->Execute(*this);
}

SQLRETURN TStatement::ExecuteInternal() {
    RowCount_ = 0;
    bool hasSuccessfulParamSet = false;
    bool rowCountUsable = true;
    const SQLULEN paramsetSize = ParamCount_ > 0 ? ParamBindings_.GetArraySize() : 1;
    SQLUSMALLINT* const operations = ParamBindings_.GetArrayStatusPtr();
    SQLUSMALLINT* const statuses = ImpParamBindings_.GetArrayStatusPtr();
    SQLULEN* const processed = ImpParamBindings_.GetRowsProcessedPtr();
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
    ClearResults();
    CheckCancellation();
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
        const NYdb::NRetry::TRetryOperationSettings retrySettings = MakeAutocommitRetrySettings();

        const NYdb::TStatus execStatus = client->RetryQuerySync(
            [this, &params, &affectedRows, paramSet](NQuery::TSession session) -> NYdb::TStatus {
                CheckCancellation();
                NQuery::TExecuteQueryResult result = ExecuteQuery(session, params, paramSet);
                CheckExecutionStatus(result);
                if (!result.IsSuccess()) {
                    return StatusFrom(result);
                }
                affectedRows = ExtractAffectedRows(result);
                if (!IsCancelRequested()) {
                    SetResults(result);
                }
                return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
            },
            retrySettings);

        NStatusHelpers::ThrowOnError(execStatus);
    } else {
        NQuery::TSession& session = Conn_->GetOrCreateQuerySession();
        CheckCancellation();
        NQuery::TExecuteQueryResult result = ExecuteQuery(session, params, paramSet);
        NStatusHelpers::ThrowOnError(result);
        affectedRows = ExtractAffectedRows(result);
        if (!IsCancelRequested()) {
            SetResults(result);
        }
    }
    InAtExec_ = false;
    NeedDataParam_ = 0;
    NeedDataTokenDelivered_ = false;
    AtExecValues_.clear();
    return SQL_SUCCESS;
}

SQLRETURN TStatement::MoreResults() {
    if (InAtExec_) {
        return AddError("HY010", 0, "Function sequence error");
    }
    RowCount_ = -1;
    if (!RemainingResultSets_.empty()) {
        auto cursor = CreateExecCursor(
            RemainingResultSets_.front(), Attributes_.CursorType == SQL_CURSOR_STATIC);
        SetCursor(std::move(cursor));
        RemainingResultSets_.pop_front();
        return SQL_SUCCESS;
    }
    ClearResults();
    return SQL_NO_DATA;
}

SQLUSMALLINT TStatement::FindNextNeedDataParam() const {
    for (SQLSMALLINT i = 1; i <= ParamBindings_.GetRecordCount(); ++i) {
        const TDescRecord* record = ParamBindings_.FindRecord(i);
        if (record && record->AtExec
            && (AtExecValues_.size() <= static_cast<size_t>(i) || !AtExecValues_[i].Complete)) {
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
    const NYdb::TParams& params,
    SQLULEN paramSet)
{
    const std::vector<TBoundParam> activeParams = GetBoundParams(paramSet);
    const TParamRewriteResult rewritten = RewriteOdbcSql(
        PreparedQuery_, activeParams, Attributes_.GetNoScanMode() != SQL_NOSCAN_ON);
    if (!rewritten.Success) {
        throw TOdbcException(rewritten.SqlState, 0, rewritten.Message);
    }
    const bool isDdl = StartsWithSqlStatement(
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
        CheckCancellation();
    }
    return session.ExecuteQuery(
        queryText,
        NQuery::TTxControl::Tx(*Conn_->GetTx()).CommitTx(false),
        params,
        execSettings).ExtractValueSync();
}



SQLRETURN TStatement::Fetch() {
    return FetchScroll(SQL_FETCH_NEXT, 0);
}

SQLRETURN TStatement::FetchScroll(SQLSMALLINT orientation, SQLLEN offset) {
    if (!Cursor_) {
        return SQL_NO_DATA;
    }
    switch (orientation) {
        case SQL_FETCH_NEXT:
        case SQL_FETCH_PRIOR:
        case SQL_FETCH_FIRST:
        case SQL_FETCH_LAST:
        case SQL_FETCH_ABSOLUTE:
        case SQL_FETCH_RELATIVE:
            break;
        default:
            return AddError("HY106", 0, "Fetch type out of range");
    }
    if (Attributes_.CursorType == SQL_CURSOR_FORWARD_ONLY
        && orientation != SQL_FETCH_NEXT) {
        return AddError("HY106", 0, "Fetch type out of range for a forward-only cursor");
    }

    RefreshRowBindings();
    const SQLULEN rowArraySize = RowBindings_.GetArraySize();
    SQLUSMALLINT* const statuses = ImpRowBindings_.GetArrayStatusPtr();
    SQLULEN* const fetched = ImpRowBindings_.GetRowsProcessedPtr();
    if (fetched) {
        *fetched = 0;
    }
    if (statuses) {
        std::fill_n(statuses, rowArraySize, SQL_ROW_NOROW);
    }

    const TFetchResult fetch = Cursor_->Fetch(
        orientation, offset, rowArraySize, Attributes_.GetMaxRows());
    if (fetch.Rows == 0) {
        GetDataOffsets_.clear();
        return SQL_NO_DATA;
    }

    SQLRETURN result = SQL_SUCCESS;
    for (SQLULEN row = 0; row < fetch.Rows; ++row) {
        const SQLULEN rows = row + 1;
        const SQLRETURN rowResult = FillBoundColumns(row);
        if (fetched) {
            *fetched = rows;
        }
        if (statuses) {
            statuses[row] = rowResult == SQL_SUCCESS_WITH_INFO
                ? SQL_ROW_SUCCESS_WITH_INFO
                : rowResult == SQL_SUCCESS ? SQL_ROW_SUCCESS : SQL_ROW_ERROR;
        }
        if (rowResult == SQL_ERROR) {
            result = SQL_ERROR;
        } else if (rowResult == SQL_SUCCESS_WITH_INFO && result == SQL_SUCCESS) {
            result = SQL_SUCCESS_WITH_INFO;
        }
    }
    GetDataOffsets_.assign(Cursor_->GetColumnMeta().size(), 0);
    if (fetch.OverlappedStart) {
        AddError("01S06", 0, "Attempt to fetch before the result set returned the first rowset",
                 SQL_SUCCESS_WITH_INFO);
        if (result == SQL_SUCCESS) {
            result = SQL_SUCCESS_WITH_INFO;
        }
    }
    return result;
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
        0, columnNumber, targetType, targetValue, bufferLength, strLenOrInd,
        &GetDataOffsets_[columnNumber - 1]);
    if (const char* sqlState = ConsumeLastConvertSqlState()) {
        AddError(sqlState, 0, std::strcmp(sqlState, "22003") == 0 ? "Numeric value out of range" : "Conversion error");
    }
    return rc;
}

SQLRETURN TStatement::FillBoundColumns(SQLULEN row) {
    if (!Cursor_) {
        return SQL_NO_DATA;
    }
    SQLRETURN result = SQL_SUCCESS;
    for (SQLSMALLINT number = 1; number <= RowBindings_.GetRecordCount(); ++number) {
        const TDescRecord* col = RowBindings_.FindRecord(number);
        if (!col || !col->DataPtr) {
            continue;
        }
        if (static_cast<size_t>(number) > Cursor_->GetColumnMeta().size()) {
            if (row == 0) {
                AddError("07009", 0, "Invalid descriptor index");
            }
            result = SQL_ERROR;
            continue;
        }
        const TResolvedBinding binding = RowBindings_.ResolveBinding(*col, row);
        SQLLEN* indicator = binding.Indicator;
        SQLLEN* length = binding.OctetLength;
        SQLLEN convertedLength = 0;
        SQLRETURN rc = Cursor_->GetData(
            row, static_cast<SQLUSMALLINT>(number), col->Type,
            binding.Data, col->OctetLength,
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
            if (result == SQL_SUCCESS) {
                result = SQL_SUCCESS_WITH_INFO;
            }
        } else if (rc != SQL_SUCCESS) {
            if (const char* sqlState = ConsumeLastConvertSqlState()) {
                AddError(sqlState, 0, std::strcmp(sqlState, "22003") == 0 ? "Numeric value out of range" : "Conversion error");
            }
            result = rc;
        }
    }
    return result;
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

    GetAppRowDesc().WithLock([&](TDescriptor& desc) {
        if (!targetValue) {
            desc.RemoveRecord(static_cast<SQLSMALLINT>(columnNumber));
            return;
        }
        TDescRecord& record = desc.Record(static_cast<SQLSMALLINT>(columnNumber));
        record.Type = targetType;
        record.Length = bufferLength;
        record.OctetLength = bufferLength;
        record.DataPtr = targetValue;
        record.IndicatorPtr = strLenOrInd;
        record.OctetLengthPtr = strLenOrInd;
    });
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
    if (paramNumber < 1) {
        throw TOdbcException("07009", 0, "Invalid descriptor index");
    }

    const bool atExec = strLenOrIndPtr
        && (*strLenOrIndPtr == SQL_DATA_AT_EXEC
            || *strLenOrIndPtr <= SQL_LEN_DATA_AT_EXEC_OFFSET);

    GetAppParamDesc().WithLock(ImpParamDesc_, [&](TDescriptor& appDesc, TDescriptor& impDesc) {
        if (!parameterValuePtr && !strLenOrIndPtr) {
            appDesc.RemoveRecord(static_cast<SQLSMALLINT>(paramNumber));
            impDesc.RemoveRecord(static_cast<SQLSMALLINT>(paramNumber));
            return;
        }
        TDescRecord& app = appDesc.Record(static_cast<SQLSMALLINT>(paramNumber));
        app.Type = valueType;
        app.Length = bufferLength;
        app.OctetLength = bufferLength;
        app.DataPtr = parameterValuePtr;
        app.IndicatorPtr = strLenOrIndPtr;
        app.OctetLengthPtr = strLenOrIndPtr;
        app.ParameterType = inputOutputType;
        app.AtExec = atExec;

        TDescRecord& imp = impDesc.Record(static_cast<SQLSMALLINT>(paramNumber));
        imp.Type = parameterType;
        imp.Length = static_cast<SQLLEN>(columnSize);
        imp.OctetLength = static_cast<SQLLEN>(columnSize);
        imp.Precision = static_cast<SQLSMALLINT>(columnSize);
        imp.Scale = decimalDigits;
        imp.Nullable = SQL_NULLABLE;
        imp.ParameterType = inputOutputType;
    });
    if (!parameterValuePtr && !strLenOrIndPtr) {
        if (AtExecValues_.size() > paramNumber) {
            AtExecValues_[paramNumber] = {};
        }
    } else {
        if (AtExecValues_.size() <= paramNumber) {
            AtExecValues_.resize(paramNumber + 1);
        }
        AtExecValues_[paramNumber] = {};
    }

    InvalidatePreparedColumnMeta();
    return SQL_SUCCESS;
}

std::vector<TBoundParam> TStatement::GetBoundParams(SQLULEN paramSet) const {
    std::vector<TBoundParam> params;
    for (SQLSMALLINT number = 1; number <= ParamCount_; ++number) {
        const TDescRecord* app = ParamBindings_.FindRecord(number);
        const TDescRecord* imp = ImpParamBindings_.FindRecord(number);
        if (!app || !imp) {
            continue;
        }
        const TResolvedBinding binding = ParamBindings_.ResolveBinding(*app, paramSet);
        SQLLEN* lengthOrIndicator = app->IndicatorPtr == app->OctetLengthPtr
            ? binding.Indicator
            : binding.OctetLength;
        const bool isNullData = binding.Indicator
            && *binding.Indicator == SQL_NULL_DATA;
        const bool atExecNullData = app->AtExec
            && AtExecValues_.size() > static_cast<size_t>(number)
            && AtExecValues_[number].Complete
            && AtExecValues_[number].Indicator == SQL_NULL_DATA;
        TBoundParam param{
            static_cast<SQLUSMALLINT>(number), app->Type, imp->Type,
            static_cast<SQLULEN>(imp->Length), imp->Scale, binding.Data, app->OctetLength,
            lengthOrIndicator, app->AtExec, isNullData || atExecNullData};
        if (binding.Indicator && *binding.Indicator == SQL_NULL_DATA) {
            param.StrLenOrIndPtr = binding.Indicator;
        }
        params.push_back(std::move(param));
    }
    return params;
}

SQLRETURN TStatement::BuildParams(NYdb::TParams& out, SQLULEN paramSet) {
    ClearErrors();
    NYdb::TParamsBuilder paramsBuilder;
    const auto conversionError = [&](const TBoundParam& param) {
        const char* state = ConsumeLastConvertSqlState();
        return AddError(
            state ? state : "07006",
            0,
            "Invalid ODBC parameter " + std::to_string(param.ParamNumber)
                + " in parameter set " + std::to_string(paramSet + 1)
                + " (C type " + std::to_string(static_cast<int>(param.ValueType))
                + ", SQL type " + std::to_string(static_cast<int>(param.ParameterType)) + ")");
    };
    for (const TBoundParam& param : GetBoundParams(paramSet)) {
        const std::string paramName = "$p" + std::to_string(param.ParamNumber);
        if (param.AtExec) {
            if (AtExecValues_.size() <= param.ParamNumber
                || !AtExecValues_[param.ParamNumber].Complete) {
                return AddError("HY000", 0, "Missing data-at-execution parameter value");
            }
            TAtExecValue& value = AtExecValues_[param.ParamNumber];
            SQLLEN indicator = value.Indicator == SQL_NULL_DATA
                ? SQL_NULL_DATA
                : SQL_NTS;
            TBoundParam tmp = param;
            tmp.ParameterValuePtr = value.Data.data();
            tmp.StrLenOrIndPtr = &indicator;
            const bool optional = BoundParamIsNull(tmp)
                || GetDeclaredParamOptionality(PreparedQuery_, param.ParamNumber).value_or(false);
            const SQLRETURN convRc = ConvertParam(
                tmp, paramsBuilder.AddParam(paramName), optional);
            if (convRc != SQL_SUCCESS) {
                return conversionError(param);
            }
            continue;
        }
        const bool optional = BoundParamIsNull(param)
            || GetDeclaredParamOptionality(PreparedQuery_, param.ParamNumber).value_or(false);
        const SQLRETURN convRc = ConvertParam(
            param, paramsBuilder.AddParam(paramName), optional);
        if (convRc != SQL_SUCCESS) {
            return conversionError(param);
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
    RowCount_ = -1;
    PreparedQuery_.clear();
    IsPrepared_ = false;
    ParamCount_ = 0;
    PreparedColumnMeta_.reset();
    ClearResults();
}

SQLRETURN TStatement::DescribeParam(SQLUSMALLINT paramNumber, SQLSMALLINT* dataTypePtr, SQLULEN* paramSizePtr,
                                    SQLSMALLINT* decimalDigitsPtr, SQLSMALLINT* nullablePtr) {
    if (!IsPrepared_) {
        throw TOdbcException("HY010", 0, "Function sequence error");
    }
    if (paramNumber < 1 || paramNumber > ParamCount_) {
        throw TOdbcException("07009", 0, "Invalid descriptor index");
    }
    RefreshParamBindings();
    const TDescRecord* record = ImpParamBindings_.FindRecord(static_cast<SQLSMALLINT>(paramNumber));
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
    RefreshParamBindings();
    if (NeedDataParam_ != 0 && NeedDataTokenDelivered_) {
        if (AtExecValues_.size() <= NeedDataParam_) {
            AtExecValues_.resize(NeedDataParam_ + 1);
        }
        AtExecValues_[NeedDataParam_].Complete = true;
        NeedDataParam_ = 0;
        NeedDataTokenDelivered_ = false;
    }
    const SQLUSMALLINT next = FindNextNeedDataParam();
    if (next != 0) {
        NeedDataParam_ = next;
        NeedDataTokenDelivered_ = true;
        *valuePtr = ParamBindings_.FindRecord(static_cast<SQLSMALLINT>(next))->DataPtr;
        return SQL_NEED_DATA;
    }
    InAtExec_ = false;
    NeedDataParam_ = 0;
    return Conn_->Execute(*this);
}

SQLRETURN TStatement::PutData(SQLPOINTER data, SQLLEN strLenOrInd) {
    if (!InAtExec_ || NeedDataParam_ == 0) {
        throw TOdbcException("HY010", 0, "Function sequence error");
    }
    RefreshParamBindings();
    const TDescRecord* param = ParamBindings_.FindRecord(
        static_cast<SQLSMALLINT>(NeedDataParam_));
    if (!param || !NeedDataTokenDelivered_) {
        throw TOdbcException("HY010", 0, "Function sequence error");
    }
    if (AtExecValues_.size() <= NeedDataParam_) {
        AtExecValues_.resize(NeedDataParam_ + 1);
    }
    TAtExecValue& value = AtExecValues_[NeedDataParam_];
    SQLLEN chunkLen = strLenOrInd;
    if (chunkLen == SQL_NULL_DATA) {
        value.Indicator = SQL_NULL_DATA;
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
        value.Data.append(static_cast<const char*>(data), static_cast<size_t>(chunkLen));
    }
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Cancel() {
    if (IsRetired()) {
        return SQL_INVALID_HANDLE;
    }
    try {
        if (const auto result = CancelExecuting()) {
            return *result;
        }
        std::shared_lock lifecycle(*GetLifecycle(), std::try_to_lock);
        if (!lifecycle.owns_lock()) {
            return CancelExecuting().value_or(SQL_SUCCESS);
        }
        std::unique_lock operation(OperationMutex_, std::try_to_lock);
        if (!operation.owns_lock()) {
            return CancelExecuting().value_or(SQL_SUCCESS);
        }
        if (IsRetired()) {
            return SQL_INVALID_HANDLE;
        }
        ClearErrors();
        BeforeCall();
        if (Cursor_ || !RemainingResultSets_.empty() || InAtExec_) {
            ClearResults();
            InAtExec_ = false;
            NeedDataParam_ = 0;
            NeedDataTokenDelivered_ = false;
            AtExecValues_.clear();
        }
        SetLastReturnCode(SQL_SUCCESS);
        return SQL_SUCCESS;
    } catch (...) {
        // Cross-thread cancellation must not replace the executing call's diagnostics.
        return SQL_ERROR;
    }
}

void TStatement::StartExecution() {
    ExecutionState_.store(EExecutionState::Executing, std::memory_order_relaxed);
}

void TStatement::CheckCancellation() const {
    if (IsCancelRequested()) {
        throw TOdbcException("HY008", 0, "Operation canceled");
    }
}

void TStatement::CheckExecutionStatus(const TStatus& status) {
    if (IsCancelRequested() && !status.IsSuccess()) {
        // A requested cancel must stop retries without hiding other errors,
        // particularly an unknown commit outcome.
        NStatusHelpers::ThrowOnError(status);
    }
}

std::optional<SQLRETURN> TStatement::CancelExecuting() {
    auto expected = EExecutionState::Executing;
    if (ExecutionState_.compare_exchange_strong(expected, EExecutionState::CancelRequested,
            std::memory_order_relaxed) || expected == EExecutionState::CancelRequested) {
        return SQL_SUCCESS;
    }
    return std::nullopt;
}

void TStatement::FinishExecution() noexcept {
    const bool requested = ExecutionState_.exchange(EExecutionState::Idle, std::memory_order_relaxed)
        == EExecutionState::CancelRequested;
    if (requested) {
        try {
            ClearResults();
        } catch (const std::bad_alloc&) {
            // Disposal already cleared the cursor; don't replace the original diagnostics.
            InvalidatePreparedColumnMeta();
        }
    }
}

SQLRETURN TStatement::SetCursorName(const std::string& name) {
    CursorName_ = name;
    return SQL_SUCCESS;
}

SQLRETURN TStatement::GetCursorName(SQLCHAR* name, SQLSMALLINT bufferLength, SQLSMALLINT* nameLengthPtr) {
    return Diag::WriteOdbcString(*this, CursorName_, name, bufferLength, nameLengthPtr);
}


SQLRETURN TStatement::Close(bool force) {
    if (!force && !Cursor_ && RemainingResultSets_.empty()) {
        throw TOdbcException("24000", 0, "Invalid handle");
    }

    ClearResults();
    ClearErrors();
    return SQL_SUCCESS;
}

void TStatement::UnbindColumns() {
    GetAppRowDesc().Clear();
}

void TStatement::ResetParams() {
    RefreshParamBindings();
    // Resetting bindings keeps the result schema learned by execution.
    GetAppParamDesc().WithLock(ImpParamDesc_, [&](TDescriptor& app, TDescriptor& imp) {
        app.ClearRecords();
        imp.ClearRecords();
        app.SnapshotUnlocked(ParamBindings_, ParamGeneration_);
        imp.SnapshotUnlocked(ImpParamBindings_, ImpParamGeneration_);
    });
    AtExecValues_.clear();
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
    const auto& columns = GetColumnMeta();
    *colCount = static_cast<SQLSMALLINT>(columns.size());
    return SQL_SUCCESS;
}

const std::vector<TColumnMeta>& TStatement::GetColumnMeta() {
    if (Cursor_) {
        return Cursor_->GetColumnMeta();
    }
    EnsurePreparedColumnMeta();
    return *PreparedColumnMeta_;
}

void TStatement::EnsurePreparedColumnMeta() {
    RefreshParamBindings();
    if (PreparedColumnMeta_) {
        return;
    }
    if (!IsPrepared_) {
        throw TOdbcException("HY010", 0, "Function sequence error");
    }
    if (HasMultipleSqlStatements(PreparedQuery_)) {
        throw TOdbcException(
            "HYC00", 0,
            "Result metadata before execution is unavailable for multiple statements");
    }

    if (!StartsWithSqlStatement(PreparedQuery_, {"SELECT"})) {
        if (StartsWithSqlStatement(
                PreparedQuery_,
                {"INSERT", "UPDATE", "DELETE", "UPSERT", "REPLACE", "MERGE",
                 "CREATE", "DROP", "ALTER", "GRANT", "REVOKE", "COMMIT", "ROLLBACK"})) {
            PreparedColumnMeta_.emplace();
            SetImpRowDesc(*PreparedColumnMeta_);
            return;
        }
        throw TOdbcException(
            "HYC00", 0,
            "Result metadata before execution is supported only for SELECT statements");
    }

    // Inferring parameter result types from application buffers would make
    // metadata value-dependent and could read data-at-execution values before
    // SQLExecute. Keep this fallback deliberately limited to parameterless
    // SELECT statements until YDB exposes a compile-only result-schema API.
    if (ParamCount_ != 0) {
        throw TOdbcException(
            "HYC00", 0,
            "Result metadata before execution is unavailable for parameterized statements");
    }

    auto client = Conn_->GetClient();
    if (!client) {
        throw TOdbcException("HY000", 0, "No client connection");
    }

    const NYdb::TParams params = NYdb::TParamsBuilder().Build();
    const std::vector<TBoundParam> activeParams;
    const TParamRewriteResult rewritten = RewriteOdbcSql(
        BuildResultMetadataQuery(PreparedQuery_), activeParams,
        Attributes_.GetNoScanMode() != SQL_NOSCAN_ON);
    if (!rewritten.Success) {
        throw TOdbcException(rewritten.SqlState, 0, rewritten.Message);
    }
    const std::string queryText = Conn_->WrapQueryForCurrentCatalog(rewritten.Sql);

    NYdb::NRetry::TRetryOperationSettings retrySettings;
    retrySettings.Idempotent(true);
    const SQLUINTEGER queryTimeoutSec = Attributes_.GetQueryTimeoutSec();
    if (queryTimeoutSec > 0) {
        const TDuration deadline = TDuration::Seconds(queryTimeoutSec);
        retrySettings.MaxTimeout(deadline).GetSessionClientTimeout(deadline);
    }

    std::optional<std::vector<TColumnMeta>> columns;
    StartExecution();
    Y_DEFER { FinishExecution(); };
    const NYdb::TStatus execStatus = client->RetryQuerySync(
        [this, &queryText, &params, &columns, queryTimeoutSec](
            NQuery::TSession session) -> NYdb::TStatus {
            CheckCancellation();
            NQuery::TExecuteQuerySettings execSettings;
            execSettings.SchemaInclusionMode(NQuery::ESchemaInclusionMode::Always);
            if (queryTimeoutSec > 0) {
                execSettings.ClientTimeout(TDuration::Seconds(queryTimeoutSec));
            }
            NQuery::TExecuteQueryResult result = session.ExecuteQuery(
                queryText,
                NQuery::TTxControl::NoTx(),
                params,
                execSettings).ExtractValueSync();
            CheckExecutionStatus(result);
            if (!result.IsSuccess()) {
                return StatusFrom(result);
            }
            if (result.GetResultSets().size() != 1) {
                return NYdb::TStatus(
                    EStatus::BAD_REQUEST,
                    NYdb::NIssue::TIssues{NYdb::NIssue::TIssue(
                        "Result metadata query did not return exactly one result set")});
            }
            const auto metadataCursor = CreateExecCursor(result.GetResultSet(0), false);
            columns = metadataCursor->GetColumnMeta();
            return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
        },
        retrySettings);
    NStatusHelpers::ThrowOnError(execStatus);
    if (!columns) {
        throw TOdbcException("HY000", 0, "Result metadata is unavailable");
    }
    PreparedColumnMeta_ = std::move(*columns);
    SetImpRowDesc(*PreparedColumnMeta_);
}

void TStatement::InvalidatePreparedColumnMeta() {
    PreparedColumnMeta_.reset();
    if (!Cursor_) {
        ImpRowDesc_.Clear();
    }
}

void TStatement::SetCursor(std::unique_ptr<ICursor> cursor) {
    Cursor_ = std::move(cursor);
    GetDataOffsets_.clear();
    static const std::vector<TColumnMeta> EmptyColumns;
    SetImpRowDesc(Cursor_ ? Cursor_->GetColumnMeta()
                         : PreparedColumnMeta_ ? *PreparedColumnMeta_ : EmptyColumns);
}

void TStatement::SetImpRowDesc(const std::vector<TColumnMeta>& columns) {
    ImpRowDesc_.WithLock([&](TDescriptor& desc) {
        desc.ClearRecords();
        SQLSMALLINT number = 0;
        for (const TColumnMeta& column : columns) {
            TDescRecord& record = desc.Record(++number);
            record.Name = column.Name;
            record.Type = column.SqlType;
            record.Length = static_cast<SQLLEN>(column.Size);
            record.OctetLength = static_cast<SQLLEN>(column.Size);
            record.Precision = static_cast<SQLSMALLINT>(column.Size);
            record.Scale = column.DecimalDigits;
            record.Nullable = column.Nullable;
        }
    });
}

void TStatement::ClearResults() {
    RemainingResultSets_.clear();
    SetCursor(nullptr);
}

void TStatement::SetResults(const NQuery::TExecuteQueryResult& result) {
    const auto& sets = result.GetResultSets();
    auto cursor = sets.empty() ? nullptr : CreateExecCursor(
        sets.front(), Attributes_.CursorType == SQL_CURSOR_STATIC);
    if (cursor && IsPrepared_) {
        PreparedColumnMeta_ = cursor->GetColumnMeta();
    }
    if (!sets.empty()) {
        RemainingResultSets_.assign(sets.begin() + 1, sets.end());
    }
    SetCursor(std::move(cursor));
}

std::optional<TStatement::TDescriptorAttribute> TStatement::ResolveDescriptorAttribute(
    SQLINTEGER attr) {
    switch (attr) {
        case SQL_ATTR_PARAM_BIND_TYPE:
            return TDescriptorAttribute{&GetAppParamDesc(), SQL_DESC_BIND_TYPE};
        case SQL_ATTR_PARAMSET_SIZE:
            return TDescriptorAttribute{&GetAppParamDesc(), SQL_DESC_ARRAY_SIZE};
        case SQL_ATTR_PARAM_BIND_OFFSET_PTR:
            return TDescriptorAttribute{&GetAppParamDesc(), SQL_DESC_BIND_OFFSET_PTR};
        case SQL_ATTR_PARAM_OPERATION_PTR:
            return TDescriptorAttribute{&GetAppParamDesc(), SQL_DESC_ARRAY_STATUS_PTR};
        case SQL_ATTR_PARAM_STATUS_PTR:
            return TDescriptorAttribute{&ImpParamDesc_, SQL_DESC_ARRAY_STATUS_PTR};
        case SQL_ATTR_PARAMS_PROCESSED_PTR:
            return TDescriptorAttribute{&ImpParamDesc_, SQL_DESC_ROWS_PROCESSED_PTR};
        case SQL_ATTR_ROW_BIND_TYPE:
            return TDescriptorAttribute{&GetAppRowDesc(), SQL_DESC_BIND_TYPE};
        case SQL_ATTR_ROW_ARRAY_SIZE:
            return TDescriptorAttribute{&GetAppRowDesc(), SQL_DESC_ARRAY_SIZE};
        case SQL_ATTR_ROW_BIND_OFFSET_PTR:
            return TDescriptorAttribute{&GetAppRowDesc(), SQL_DESC_BIND_OFFSET_PTR};
        case SQL_ATTR_ROW_STATUS_PTR:
            return TDescriptorAttribute{&ImpRowDesc_, SQL_DESC_ARRAY_STATUS_PTR};
        case SQL_ATTR_ROWS_FETCHED_PTR:
            return TDescriptorAttribute{&ImpRowDesc_, SQL_DESC_ROWS_PROCESSED_PTR};
        default:
            return std::nullopt;
    }
}

SQLRETURN TStatement::SetStmtAttr(
    SQLINTEGER attr,
    SQLPOINTER value,
    SQLINTEGER /*stringLength*/) {
    if (attr == SQL_ATTR_APP_ROW_DESC || attr == SQL_ATTR_APP_PARAM_DESC) {
        auto owner = std::dynamic_pointer_cast<TDescriptor>(PinHandle(value));
        if (value && (!owner || owner->IsRetired())) {
            return AddError("HY024", 0, "Invalid descriptor handle");
        }
        TDescriptor* desc = owner.get();
        if (desc && (desc->GetDescType() != EDescType::Explicit
                     || desc->GetConnection() != Conn_)) {
            return AddError("HY024", 0, "Descriptor belongs to another connection");
        }
        auto& current = attr == SQL_ATTR_APP_ROW_DESC ? AppRowOwner_ : AppParamOwner_;
        if (current != owner) {
            current = std::move(owner);
            if (attr == SQL_ATTR_APP_ROW_DESC) {
                RowGeneration_ = 0;
            } else {
                ParamGeneration_ = 0;
                AtExecValues_.clear();
                InvalidatePreparedColumnMeta();
            }
        }
        return SQL_SUCCESS;
    }
    if ((attr == SQL_ATTR_PARAMSET_SIZE || attr == SQL_ATTR_ROW_ARRAY_SIZE)
        && ReadIntegerAttr<SQLULEN>(value) == 0) {
        return Diag::AddInvalidAttrValue(
            *this,
            attr == SQL_ATTR_PARAMSET_SIZE ? "SQL_ATTR_PARAMSET_SIZE" : "SQL_ATTR_ROW_ARRAY_SIZE");
    }
    if (auto descriptorAttr = ResolveDescriptorAttribute(attr)) {
        return descriptorAttr->Descriptor->WithLock([&](TDescriptor& desc) {
            return desc.SetDescField(0, descriptorAttr->Field, value, 0);
        });
    }
    const auto setCursorType = [&](SQLULEN cursorType, std::string_view name) -> SQLRETURN {
        if (Cursor_) {
            return AddError("24000", 0, std::string(name) + " cannot be changed while a cursor is open");
        }
        if (cursorType == SQL_CURSOR_FORWARD_ONLY || cursorType == SQL_CURSOR_STATIC) {
            Attributes_.CursorType = cursorType;
            return SQL_SUCCESS;
        }
        if (cursorType == SQL_CURSOR_KEYSET_DRIVEN || cursorType == SQL_CURSOR_DYNAMIC) {
            Attributes_.CursorType = SQL_CURSOR_STATIC;
            return AddError("01S02", 0, std::string(name) + " was changed to static",
                            SQL_SUCCESS_WITH_INFO);
        }
        return Diag::AddInvalidAttrValue(*this, name);
    };
    switch (attr) {
        case SQL_ATTR_QUERY_TIMEOUT:
            return SetCheckedAttribute<SQLINTEGER>(
                value, Attributes_.QueryTimeoutSec, *this, "SQL_ATTR_QUERY_TIMEOUT",
                [](SQLINTEGER input) { return input >= 0; });
        case SQL_ATTR_MAX_ROWS:
            return SetCheckedAttribute<SQLLEN>(
                value, Attributes_.MaxRows, *this, "SQL_ATTR_MAX_ROWS",
                [](SQLLEN input) { return input >= 0; });
        case SQL_ATTR_NOSCAN: {
            const SQLULEN previous = Attributes_.NoScan;
            const SQLRETURN result = SetCheckedAttribute<SQLULEN>(
                value, Attributes_.NoScan, *this, "SQL_ATTR_NOSCAN",
                [](SQLULEN input) {
                    return input == SQL_NOSCAN_OFF || input == SQL_NOSCAN_ON;
                });
            if (result == SQL_SUCCESS && Attributes_.NoScan != previous) {
                InvalidatePreparedColumnMeta();
            }
            return result;
        }
        case SQL_ATTR_METADATA_ID:
            return SetCheckedAttribute<SQLULEN>(
                value, Attributes_.MetadataId, *this, "SQL_ATTR_METADATA_ID",
                [](SQLULEN input) { return input == SQL_FALSE || input == SQL_TRUE; });
        case SQL_ATTR_CONCURRENCY: {
            if (Cursor_) {
                return AddError(
                    "24000", 0,
                    "SQL_ATTR_CONCURRENCY cannot be changed while a cursor is open");
            }
            const SQLULEN concurrency = ReadIntegerAttr<SQLULEN>(value);
            if (concurrency == SQL_CONCUR_READ_ONLY) {
                Attributes_.Concurrency = concurrency;
                return SQL_SUCCESS;
            }
            if (concurrency == SQL_CONCUR_LOCK
                || concurrency == SQL_CONCUR_ROWVER
                || concurrency == SQL_CONCUR_VALUES) {
                Attributes_.Concurrency = SQL_CONCUR_READ_ONLY;
                return AddError(
                    "01S02", 0,
                    "SQL_ATTR_CONCURRENCY was changed to read-only",
                    SQL_SUCCESS_WITH_INFO);
            }
            return Diag::AddInvalidAttrValue(*this, "SQL_ATTR_CONCURRENCY");
        }
        case SQL_ATTR_CURSOR_TYPE:
            return setCursorType(
                ReadIntegerAttr<SQLULEN>(value), "SQL_ATTR_CURSOR_TYPE");
        case SQL_ATTR_CURSOR_SCROLLABLE: {
            const SQLULEN scrollable = ReadIntegerAttr<SQLULEN>(value);
            if (scrollable != SQL_NONSCROLLABLE && scrollable != SQL_SCROLLABLE) {
                return Diag::AddInvalidAttrValue(*this, "SQL_ATTR_CURSOR_SCROLLABLE");
            }
            return setCursorType(
                scrollable == SQL_NONSCROLLABLE
                    ? SQL_CURSOR_FORWARD_ONLY
                    : SQL_CURSOR_STATIC,
                "SQL_ATTR_CURSOR_SCROLLABLE");
        }
        case SQL_ATTR_CURSOR_SENSITIVITY: {
            const SQLULEN sensitivity = ReadIntegerAttr<SQLULEN>(value);
            if (sensitivity == SQL_UNSPECIFIED) {
                return SQL_SUCCESS;
            }
            if (sensitivity == SQL_INSENSITIVE) {
                return setCursorType(SQL_CURSOR_STATIC, "SQL_ATTR_CURSOR_SENSITIVITY");
            }
            if (sensitivity == SQL_SENSITIVE) {
                const SQLRETURN result = setCursorType(
                    SQL_CURSOR_STATIC, "SQL_ATTR_CURSOR_SENSITIVITY");
                return result == SQL_SUCCESS
                    ? AddError("01S02", 0,
                               "SQL_ATTR_CURSOR_SENSITIVITY was changed to insensitive",
                               SQL_SUCCESS_WITH_INFO)
                    : result;
            }
            return Diag::AddInvalidAttrValue(*this, "SQL_ATTR_CURSOR_SENSITIVITY");
        }
        case SQL_ATTR_USE_BOOKMARKS: {
            const SQLULEN bookmarks = ReadIntegerAttr<SQLULEN>(value);
            if (bookmarks == SQL_UB_OFF) {
                return SQL_SUCCESS;
            }
            if (bookmarks == SQL_UB_ON || bookmarks == SQL_UB_VARIABLE) {
                return AddError("01S02", 0, "SQL_ATTR_USE_BOOKMARKS was changed to off",
                                SQL_SUCCESS_WITH_INFO);
            }
            return Diag::AddInvalidAttrValue(*this, "SQL_ATTR_USE_BOOKMARKS");
        }
        default:
            return Diag::AddNotImplemented(*this);
    }
}

SQLRETURN TStatement::GetStmtAttr(
    SQLINTEGER attr,
    SQLPOINTER value,
    SQLINTEGER /*bufferLength*/,
    SQLINTEGER* stringLengthPtr) {
    if (!value) {
        return AddError("HY009", 0, "Invalid use of null pointer");
    }
    switch (attr) {
        case SQL_ATTR_APP_ROW_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = &GetAppRowDesc();
            return SQL_SUCCESS;
        case SQL_ATTR_APP_PARAM_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = &GetAppParamDesc();
            return SQL_SUCCESS;
        case SQL_ATTR_IMP_ROW_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = &ImpRowDesc_;
            return SQL_SUCCESS;
        case SQL_ATTR_IMP_PARAM_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = &ImpParamDesc_;
            return SQL_SUCCESS;
        default:
            break;
    }
    if (auto descriptorAttr = ResolveDescriptorAttribute(attr)) {
        return descriptorAttr->Descriptor->WithLock([&](TDescriptor& desc) {
            return desc.GetDescField(0, descriptorAttr->Field, value, 0, nullptr);
        });
    }
    if (stringLengthPtr) {
        *stringLengthPtr = 0;
    }
    if (attr == SQL_ATTR_ROW_NUMBER) {
        *static_cast<SQLULEN*>(value) = Cursor_ ? Cursor_->GetRowNumber() : 0;
        return SQL_SUCCESS;
    }
    if (attr == SQL_ATTR_CURSOR_SCROLLABLE) {
        *static_cast<SQLULEN*>(value) = Attributes_.CursorType == SQL_CURSOR_FORWARD_ONLY
            ? SQL_NONSCROLLABLE
            : SQL_SCROLLABLE;
        return SQL_SUCCESS;
    }
    if (attr == SQL_ATTR_CURSOR_SENSITIVITY) {
        *static_cast<SQLULEN*>(value) = Attributes_.CursorType == SQL_CURSOR_FORWARD_ONLY
            ? SQL_UNSPECIFIED
            : SQL_INSENSITIVE;
        return SQL_SUCCESS;
    }
    if (attr == SQL_ATTR_USE_BOOKMARKS) {
        *static_cast<SQLULEN*>(value) = SQL_UB_OFF;
        return SQL_SUCCESS;
    }
    if (auto result = TDirectAttributes::Get(attr, Attributes_, value)) {
        return *result;
    }
    return Diag::AddNotImplemented(*this);
}

SQLRETURN TStatement::GetDiagField(
    SQLSMALLINT recNumber,
    SQLSMALLINT diagIdentifier,
    SQLPOINTER diagInfoPtr,
    SQLSMALLINT bufferLength,
    SQLSMALLINT* stringLengthPtr) {
    if (diagIdentifier == SQL_DIAG_ROW_COUNT) {
        return RowCount(static_cast<SQLLEN*>(diagInfoPtr));
    }
    return TErrorManager::GetDiagField(recNumber, diagIdentifier, diagInfoPtr, bufferLength, stringLengthPtr);
}

} // namespace NYdb::NOdbc
