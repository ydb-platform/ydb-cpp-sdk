#pragma once

#include "bindings.h"

#include <string>
#include <string_view>
#include <vector>

namespace NYdb::NOdbc {

struct TParamRewriteResult {
    std::string Sql;
    bool Success = true;
    std::string SqlState;
    std::string Message;
};

TParamRewriteResult RewriteOdbcQuestionMarks(
    std::string_view sql,
    const std::vector<TBoundParam>& boundParams);

SQLSMALLINT CountOdbcParams(std::string_view sql);

} // namespace NYdb::NOdbc
