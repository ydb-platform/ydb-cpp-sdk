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

std::string RewriteOdbcEscapes(const std::string& sql);

TParamRewriteResult RewriteOdbcSql(
    std::string_view sql,
    const std::vector<TBoundParam>& boundParams,
    bool rewriteEscapes);

std::optional<bool> GetDeclaredParamOptionality(
    std::string_view sql,
    SQLUSMALLINT paramNumber);

SQLSMALLINT CountOdbcParams(std::string_view sql);

bool StartsWithSqlStatement(
    std::string_view sql,
    std::initializer_list<std::string_view> keywords);

} // namespace NYdb::NOdbc
