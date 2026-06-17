#include "statement.h"

#include "utils/convert.h"
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

namespace NYdb {
namespace NOdbc {

namespace {

    bool IsDdlQuery(const std::string& queryText) {
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
        const char* start = queryText.c_str() + i;
        const size_t remaining = queryText.size() - i;
        return StartsWithPrefix(start, remaining, "CREATE", 6) ||
               StartsWithPrefix(start, remaining, "DROP", 4) ||
               StartsWithPrefix(start, remaining, "ALTER", 5) ||
               StartsWithPrefix(start, remaining, "GRANT", 5) ||
               StartsWithPrefix(start, remaining, "REVOKE", 6);
    }

}

TStatement::TStatement(TConnection* conn)
    : Conn_(conn)
    , AppRowDesc_(std::make_unique<TDescriptor>(EDescType::AppRow, this))
    , AppParamDesc_(std::make_unique<TDescriptor>(EDescType::AppParam, this))
    , ImpRowDesc_(std::make_unique<TDescriptor>(EDescType::ImpRow, this))
    , ImpParamDesc_(std::make_unique<TDescriptor>(EDescType::ImpParam, this)) {}

SQLRETURN TStatement::Prepare(const std::string& statementText) {
    StreamFetchError_ = false;
    RowsFetched_ = 0;
    Cursor_.reset();
    PreparedQuery_ = statementText;
    IsPrepared_ = true;
    ParamCount_ = CountOdbcParams(PreparedQuery_);
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Execute() {
    if (!IsPrepared_ || PreparedQuery_.empty()) {
        throw TOdbcException("HY007", 0, "No prepared statement");
    }
    const SQLUSMALLINT next = FindNextNeedDataParam();
    if (next != 0) {
        NeedDataParam_ = next;
        InAtExec_ = true;
        return SQL_NEED_DATA;
    }
    InAtExec_ = false;
    NeedDataParam_ = 0;
    return ExecuteInternal();
}

SQLRETURN TStatement::ExecuteInternal() {
    StreamFetchError_ = false;
    RowsFetched_ = 0;
    Cursor_.reset();
    auto client = Conn_->GetClient();
    if (!client) {
        throw TOdbcException("HY000", 0, "No client connection");
    }
    NYdb::TParams params = NYdb::TParamsBuilder().Build();
    const SQLRETURN buildRc = BuildParams(params);
    if (buildRc != SQL_SUCCESS) {
        return buildRc;
    }

    if (Conn_->GetAutocommit()) {
        Conn_->ResetTx();
        Conn_->ResetQuerySession();
        const NYdb::NRetry::TRetryOperationSettings retrySettings = MakeAutocommitRetrySettings();

        const NYdb::TStatus execStatus = client->RetryQuerySync(
            [this, &params](NQuery::TSession session) -> NYdb::TStatus {
                auto retryIterator = CreateExecuteIterator(session, params);
                if (!retryIterator.IsSuccess()) {
                    return StatusFrom(retryIterator);
                }
                TExecCursorCreateResult created = TryCreateExecCursor(this, std::move(retryIterator));
                if (!created.Status.IsSuccess()) {
                    return created.Status;
                }
                Cursor_ = std::move(created.Cursor);
                return NYdb::TStatus(EStatus::SUCCESS, NYdb::NIssue::TIssues());
            },
            retrySettings);

        NStatusHelpers::ThrowOnError(execStatus);
    } else {
        NQuery::TSession& session = Conn_->GetOrCreateQuerySession();
        auto iterator = CreateExecuteIterator(session, params);
        NStatusHelpers::ThrowOnError(iterator);
        TExecCursorCreateResult created = TryCreateExecCursor(this, std::move(iterator));
        NStatusHelpers::ThrowOnError(created.Status);
        Cursor_ = std::move(created.Cursor);
    }
    RowCount_ = Cursor_ ? -1 : 0;
    InAtExec_ = false;
    NeedDataParam_ = 0;
    return SQL_SUCCESS;
}

SQLUSMALLINT TStatement::FindNextNeedDataParam() const {
    for (const auto& param : BoundParams_) {
        if (param.AtExec && !param.AtExecComplete) {
            return param.ParamNumber;
        }
    }
    return 0;
}

NYdb::NRetry::TRetryOperationSettings TStatement::MakeAutocommitRetrySettings() {
    NYdb::NRetry::TRetryOperationSettings settings;
    settings.Idempotent(true);
    SQLUINTEGER queryTimeoutSec = Attributes_.GetQueryTimeoutSec();
    if (queryTimeoutSec > 0) {
        const TDuration deadline = TDuration::Seconds(queryTimeoutSec);
        settings.MaxTimeout(deadline).GetSessionClientTimeout(deadline);
    }
    return settings;
}

NQuery::TExecuteQueryIterator TStatement::CreateExecuteIterator(NQuery::TSession& session, const NYdb::TParams& params) {
    const std::string sqlAfterEscapes = Attributes_.GetNoScanMode() == SQL_NOSCAN_ON
        ? PreparedQuery_
        : RewriteOdbcEscapes(PreparedQuery_);
    const TParamRewriteResult rewritten = RewriteOdbcQuestionMarks(sqlAfterEscapes, BoundParams_);
    if (!rewritten.Success) {
        throw TOdbcException(rewritten.SqlState, 0, rewritten.Message);
    }
    const bool isDdl = IsDdlQuery(rewritten.Sql);
    const std::string queryText = Conn_->WrapQueryForCurrentCatalog(rewritten.Sql);
    NQuery::TExecuteQuerySettings execSettings;
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
    if (LastFetchRc_ != SQL_SUCCESS) {
        return LastFetchRc_;
    }
    return GetLastReturnCode() == SQL_SUCCESS_WITH_INFO ? SQL_SUCCESS_WITH_INFO : SQL_SUCCESS;
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
    const SQLRETURN rc = Cursor_->GetData(columnNumber, targetType, targetValue, bufferLength, strLenOrInd);
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
    for (const auto& col : BoundColumns_) {
        const SQLRETURN rc = Cursor_->GetData(col.ColumnNumber, col.TargetType, col.TargetValue, col.BufferLength, col.StrLenOrInd);
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

    const bool atExec = strLenOrIndPtr && *strLenOrIndPtr == SQL_DATA_AT_EXEC;

    BoundParams_.erase(std::remove_if(BoundParams_.begin(), BoundParams_.end(),
        [paramNumber](const TBoundParam& p) { return p.ParamNumber == paramNumber; }), BoundParams_.end());

    if (!parameterValuePtr && !atExec) {
        return SQL_SUCCESS;
    }
    BoundParams_.push_back({paramNumber, inputOutputType, valueType, parameterType, columnSize, decimalDigits,
                            parameterValuePtr, bufferLength, strLenOrIndPtr, atExec, false, {}});
    return SQL_SUCCESS;
}

SQLRETURN TStatement::BuildParams(NYdb::TParams& out) {
    ClearErrors();
    NYdb::TParamsBuilder paramsBuilder;
    for (const auto& param : BoundParams_) {
        const std::string paramName = "$p" + std::to_string(param.ParamNumber);
        if (param.AtExec) {
            if (!param.AtExecComplete || param.AtExecChunk.empty()) {
                return AddError("HY000", 0, "Missing data-at-execution parameter value");
            }
            SQLLEN nts = SQL_NTS;
            TBoundParam tmp = param;
            tmp.ParameterValuePtr = const_cast<char*>(param.AtExecChunk.data());
            tmp.StrLenOrIndPtr = &nts;
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
    Cursor_.reset();
}

SQLRETURN TStatement::DescribeParam(SQLUSMALLINT paramNumber, SQLSMALLINT* dataTypePtr, SQLULEN* paramSizePtr,
                                    SQLSMALLINT* decimalDigitsPtr, SQLSMALLINT* nullablePtr) {
    if (!IsPrepared_) {
        throw TOdbcException("HY010", 0, "Function sequence error");
    }
    if (paramNumber < 1 || paramNumber > ParamCount_) {
        throw TOdbcException("07009", 0, "Invalid descriptor index");
    }
    const auto it = std::find_if(BoundParams_.begin(), BoundParams_.end(),
        [paramNumber](const TBoundParam& p) { return p.ParamNumber == paramNumber; });
    const SQLSMALLINT dataType = it != BoundParams_.end() ? it->ParameterType : SQL_UNKNOWN_TYPE;
    const SQLULEN paramSize = it != BoundParams_.end() ? it->ColumnSize : 0;
    const SQLSMALLINT decimalDigits = it != BoundParams_.end() ? it->DecimalDigits : 0;
    const SQLSMALLINT nullable = it != BoundParams_.end() ? SQL_NULLABLE : SQL_NULLABLE_UNKNOWN;
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
    const SQLUSMALLINT next = FindNextNeedDataParam();
    if (next != 0) {
        NeedDataParam_ = next;
        *valuePtr = reinterpret_cast<SQLPOINTER>(static_cast<intptr_t>(next));
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
    for (auto& param : BoundParams_) {
        if (param.ParamNumber != NeedDataParam_) {
            continue;
        }
        SQLLEN chunkLen = strLenOrInd;
        if (chunkLen == SQL_NTS) {
            if (!data) {
                throw TOdbcException("HY009", 0, "Invalid use of null pointer");
            }
            chunkLen = static_cast<SQLLEN>(std::strlen(static_cast<const char*>(data)));
        }
        if (chunkLen > 0 && data) {
            const char* bytes = static_cast<const char*>(data);
            param.AtExecChunk.append(bytes, static_cast<size_t>(chunkLen));
        }
        if (strLenOrInd == 0 || strLenOrInd == SQL_NTS) {
            param.AtExecComplete = true;
            NeedDataParam_ = 0;
        }
        break;
    }
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Cancel() {
    if (!Cursor_ && !InAtExec_) {
        return SQL_SUCCESS;
    }
    Cursor_.reset();
    InAtExec_ = false;
    NeedDataParam_ = 0;
    for (auto& param : BoundParams_) {
        param.AtExecComplete = false;
        param.AtExecChunk.clear();
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

    Cursor_.reset();
    RowsFetched_ = 0;
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

SQLRETURN TStatement::SetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER stringLength) {
    return Attributes_.SetStmtAttr(attr, value, stringLength, *this);
}

SQLRETURN TStatement::GetStmtAttr(SQLINTEGER attr, SQLPOINTER value, SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr) {
    if (!value) {
        return AddError("HY009", 0, "Invalid use of null pointer");
    }
    switch (attr) {
        case SQL_ATTR_APP_ROW_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = AppRowDesc_.get();
            return SQL_SUCCESS;
        case SQL_ATTR_APP_PARAM_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = AppParamDesc_.get();
            return SQL_SUCCESS;
        case SQL_ATTR_IMP_ROW_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = ImpRowDesc_.get();
            return SQL_SUCCESS;
        case SQL_ATTR_IMP_PARAM_DESC:
            *reinterpret_cast<SQLHDESC*>(value) = ImpParamDesc_.get();
            return SQL_SUCCESS;
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

} // namespace NOdbc
} // namespace NYdb
