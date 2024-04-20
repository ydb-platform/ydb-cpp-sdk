#pragma once

#include "stats.h"

#include <src/api/grpc/ydb_query_v1.grpc.pb.h>

#include <ydb-cpp-sdk/client/result/result.h>
#include <ydb-cpp-sdk/client/types/fluent_settings_helpers.h>
#include <ydb-cpp-sdk/client/types/operation/operation.h>
#include <ydb-cpp-sdk/client/types/request_settings.h>
#include <ydb-cpp-sdk/client/types/status/status.h>

#include <ydb-cpp-sdk/library/threading/future/future.h>

namespace NYdb::NQuery {

enum class ESyntax {
    Unspecified = 0,
    YqlV1 = 1, // YQL
    Pg = 2, // PostgresQL
};

enum class EExecMode {
    Unspecified = 0,
    Parse = 10,
    Validate = 20,
    Explain = 30,
    Execute = 50,
};

enum class EStatsMode {
    Unspecified = 0,
    None = 10,
    Basic = 20,
    Full = 30,
    Profile = 40,
};

std::optional<EStatsMode> ParseStatsMode(std::string_view statsMode);
std::string_view StatsModeToString(const EStatsMode statsMode);

enum class EExecStatus {
    Unspecified = 0,
    Starting = 10,
    Aborted = 20,
    Canceled = 30,
    Completed = 40,
    Failed = 50,
};

class TExecuteQueryPart;
using TAsyncExecuteQueryPart = NThreading::TFuture<TExecuteQueryPart>;

class TExecuteQueryIterator : public TStatus {
    friend class TExecQueryImpl;
public:
    class TReaderImpl;

    TAsyncExecuteQueryPart ReadNext();

private:
    TExecuteQueryIterator(
        std::shared_ptr<TReaderImpl> impl,
        TPlainStatus&& status)
    : TStatus(std::move(status))
    , ReaderImpl_(impl) {}

    std::shared_ptr<TReaderImpl> ReaderImpl_;
};

using TAsyncExecuteQueryIterator = NThreading::TFuture<TExecuteQueryIterator>;

struct TExecuteQuerySettings : public TRequestSettings<TExecuteQuerySettings> {
    FLUENT_SETTING_DEFAULT(ESyntax, Syntax, ESyntax::YqlV1);
    FLUENT_SETTING_DEFAULT(EExecMode, ExecMode, EExecMode::Execute);
    FLUENT_SETTING_DEFAULT(EStatsMode, StatsMode, EStatsMode::None);
    FLUENT_SETTING_OPTIONAL(bool, ConcurrentResultSets);
};

struct TBeginTxSettings : public TRequestSettings<TBeginTxSettings> {};
struct TCommitTxSettings : public TRequestSettings<TCommitTxSettings> {};
struct TRollbackTxSettings : public TRequestSettings<TRollbackTxSettings> {};



class TCommitTransactionResult : public TStatus {
public:
    TCommitTransactionResult(TStatus&& status);
};

class TBeginTransactionResult;

using TAsyncBeginTransactionResult = NThreading::TFuture<TBeginTransactionResult>;
using TAsyncCommitTransactionResult = NThreading::TFuture<TCommitTransactionResult>;

struct TExecuteScriptSettings : public TOperationRequestSettings<TExecuteScriptSettings> {
    FLUENT_SETTING_DEFAULT(Ydb::Query::Syntax, Syntax, Ydb::Query::SYNTAX_YQL_V1);
    FLUENT_SETTING_DEFAULT(Ydb::Query::ExecMode, ExecMode, Ydb::Query::EXEC_MODE_EXECUTE);
    FLUENT_SETTING_DEFAULT(Ydb::Query::StatsMode, StatsMode, Ydb::Query::STATS_MODE_NONE);
    FLUENT_SETTING(TDuration, ResultsTtl);
};

class TQueryContent {
public:
    TQueryContent() = default;

    TQueryContent(const std::string& text, ESyntax syntax)
        : Text(text)
        , Syntax(syntax)
    {}

    std::string Text;
    ESyntax Syntax = ESyntax::Unspecified;
};

class TScriptExecutionOperation : public TOperation {
public:
    struct TMetadata {
        std::string ExecutionId;
        EExecStatus ExecStatus = EExecStatus::Unspecified;
        EExecMode ExecMode = EExecMode::Unspecified;

        TQueryContent ScriptContent;
        Ydb::TableStats::QueryStats ExecStats;
        std::vector<Ydb::Query::ResultSetMeta> ResultSetsMeta;
    };

    using TOperation::TOperation;
    TScriptExecutionOperation(TStatus&& status, Ydb::Operations::Operation&& operation);

    const TMetadata& Metadata() const {
        return Metadata_;
    }

private:
    TMetadata Metadata_;
};

struct TFetchScriptResultsSettings : public TRequestSettings<TFetchScriptResultsSettings> {
    FLUENT_SETTING(std::string, FetchToken);
    FLUENT_SETTING_DEFAULT(ui64, RowsLimit, 1000);
};

class TFetchScriptResultsResult : public TStatus {
public:
    bool HasResultSet() const { return ResultSet_.has_value(); }
    ui64 GetResultSetIndex() const { return ResultSetIndex_; }
    const TResultSet& GetResultSet() const { return *ResultSet_; }
    TResultSet ExtractResultSet() { return std::move(*ResultSet_); }
    const std::string& GetNextFetchToken() const { return NextFetchToken_; }

    explicit TFetchScriptResultsResult(TStatus&& status)
        : TStatus(std::move(status))
    {}

    TFetchScriptResultsResult(TStatus&& status, TResultSet&& resultSet, i64 resultSetIndex, const std::string& nextFetchToken)
        : TStatus(std::move(status))
        , ResultSet_(std::move(resultSet))
        , ResultSetIndex_(resultSetIndex)
        , NextFetchToken_(nextFetchToken)
    {}

private:
    std::optional<TResultSet> ResultSet_;
    i64 ResultSetIndex_ = 0;
    std::string NextFetchToken_;
};

class TExecuteQueryResult;
using TAsyncFetchScriptResultsResult = NThreading::TFuture<TFetchScriptResultsResult>;
using TAsyncExecuteQueryResult = NThreading::TFuture<TExecuteQueryResult>;

} // namespace NYdb::NQuery
