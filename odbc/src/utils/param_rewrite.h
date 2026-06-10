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

} // namespace NYdb::NOdbc
