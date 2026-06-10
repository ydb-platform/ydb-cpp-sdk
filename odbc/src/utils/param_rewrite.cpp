#include "param_rewrite.h"
#include "sql_type_map.h"

#include <cctype>
#include <string>
#include <string_view>

namespace NYdb::NOdbc {

namespace {

bool IsParamMark(std::string_view sql, size_t i) {
    if (sql[i] != '?') {
        return false;
    }
    if (i > 0) {
        const char prev = sql[i - 1];
        if (std::isalnum(static_cast<unsigned char>(prev)) || prev == '_' || prev == ')') {
            return false;
        }
    }
    return true;
}

SQLSMALLINT FindParamType(const std::vector<TBoundParam>& params, SQLUSMALLINT index) {
    for (const auto& param : params) {
        if (param.ParamNumber == index) {
            return param.ParameterType;
        }
    }
    return 0;
}

} // namespace

TParamRewriteResult RewriteOdbcQuestionMarks(
    std::string_view sql,
    const std::vector<TBoundParam>& boundParams) {
    std::string body;
    body.reserve(sql.size());
    size_t paramCount = 0;
    bool inQuote = false;
    size_t braceDepth = 0;

    for (size_t i = 0; i < sql.size(); ++i) {
        const char ch = sql[i];
        if (inQuote) {
            body.push_back(ch);
            if (ch == '\'' && i + 1 < sql.size() && sql[i + 1] == '\'') {
                body.push_back('\'');
                ++i;
            } else if (ch == '\'') {
                inQuote = false;
            }
            continue;
        }
        if (ch == '\'') {
            inQuote = true;
            body.push_back(ch);
            continue;
        }
        if (ch == '{') {
            ++braceDepth;
            body.push_back(ch);
            continue;
        }
        if (ch == '}' && braceDepth > 0) {
            --braceDepth;
            body.push_back(ch);
            continue;
        }
        if (braceDepth == 0 && IsParamMark(sql, i)) {
            body += "$p";
            body += std::to_string(++paramCount);
            continue;
        }
        body.push_back(ch);
    }

    if (paramCount == 0) {
        return {.Sql = std::string(sql)};
    }
    if (paramCount != boundParams.size()) {
        return {.Success = false, .SqlState = "07002", .Message = "COUNT field incorrect"};
    }

    std::string declares;
    for (size_t idx = 1; idx <= paramCount; ++idx) {
        const SQLSMALLINT type = FindParamType(boundParams, static_cast<SQLUSMALLINT>(idx));
        if (!type) {
            return {.Success = false, .SqlState = "07002", .Message = "COUNT field incorrect"};
        }
        declares += "DECLARE $p" + std::to_string(idx) + " AS " + FormatYqlParamDeclareType(type) + ";\n";
    }

    return {.Sql = declares + body};
}

} // namespace NYdb::NOdbc
