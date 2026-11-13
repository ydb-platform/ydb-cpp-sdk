#pragma once

#include "bindings.h"

#include <string>
#include <string_view>
#include <initializer_list>
#include <optional>
#include <vector>

namespace NYdb::NOdbc {

struct TParamRewriteResult {
    std::string Sql;
    bool Success = true;
    std::string SqlState;
    std::string Message;
};

class TPreparedQuery {
public:
    explicit TPreparedQuery(std::string sql);
    const std::string& Text() const { return Sql_; }
    SQLSMALLINT ParameterCount() const { return ParameterCount_; }
    std::optional<bool> DeclaredOptionality(SQLUSMALLINT number) const;
    bool StartsWith(std::initializer_list<std::string_view> keywords) const;
    bool HasMultipleStatements() const { return Multiple_; }
    TParamRewriteResult Bind(const std::vector<TBoundParam>& params, bool escapes);

private:
    struct TBody {
        std::string Sql;
        size_t QuestionCount;
        std::vector<SQLUSMALLINT> Indices;
    };
    std::string Sql_;
    SQLSMALLINT ParameterCount_;
    size_t StatementOffset_;
    bool Multiple_;
    std::vector<std::optional<bool>> Declarations_;
    std::optional<TBody> Bodies_[2];
};

std::string RewriteOdbcEscapes(const std::string& sql);
std::string AddSqlPreamble(std::string_view sql, std::string_view preamble);

TParamRewriteResult RewriteOdbcSql(
    std::string_view sql,
    const std::vector<TBoundParam>& boundParams,
    bool rewriteEscapes);

std::optional<std::string> BuildParamMetadataQuery(
    std::string_view sql, bool rewriteEscapes, SQLUSMALLINT paramNumber, SQLSMALLINT paramCount);

std::string_view TrimTrailingSqlTrivia(std::string_view sql);
std::string_view GetSqlStatement(std::string_view sql);
bool HasMultipleSqlStatements(std::string_view sql);

SQLSMALLINT CountOdbcParams(std::string_view sql);

} // namespace NYdb::NOdbc
