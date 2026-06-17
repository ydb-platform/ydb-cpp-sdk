#include "param_rewrite.h"
#include "sql_type_map.h"

#include <algorithm>
#include <cctype>
#include <set>
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

bool TryParseDollarParam(std::string_view sql, size_t i, SQLUSMALLINT& index) {
    if (sql.size() < i + 3 || sql[i] != '$' || sql[i + 1] != 'p' || !std::isdigit(static_cast<unsigned char>(sql[i + 2]))) {
        return false;
    }
    unsigned n = 0;
    for (size_t j = i + 2; j < sql.size() && std::isdigit(static_cast<unsigned char>(sql[j])); ++j) {
        n = n * 10 + static_cast<unsigned>(sql[j] - '0');
    }
    index = static_cast<SQLUSMALLINT>(n);
    return true;
}

} // namespace

SQLSMALLINT CountOdbcParams(std::string_view sql) {
    SQLSMALLINT questionMarkCount = 0;
    SQLSMALLINT maxDollarIndex = 0;
    bool inQuote = false;
    size_t braceDepth = 0;

    for (size_t i = 0; i < sql.size(); ++i) {
        const char ch = sql[i];
        if (inQuote) {
            if (ch == '\'' && i + 1 < sql.size() && sql[i + 1] == '\'') {
                ++i;
            } else if (ch == '\'') {
                inQuote = false;
            }
            continue;
        }
        if (ch == '\'') {
            inQuote = true;
            continue;
        }
        if (ch == '{') {
            ++braceDepth;
            continue;
        }
        if (ch == '}' && braceDepth > 0) {
            --braceDepth;
            continue;
        }
        if (braceDepth == 0) {
            if (IsParamMark(sql, i)) {
                ++questionMarkCount;
                continue;
            }
            SQLUSMALLINT index = 0;
            if (TryParseDollarParam(sql, i, index)) {
                maxDollarIndex = std::max(maxDollarIndex, static_cast<SQLSMALLINT>(index));
            }
        }
    }

    if (questionMarkCount > 0) {
        return questionMarkCount;
    }
    return maxDollarIndex;
}

TParamRewriteResult RewriteOdbcQuestionMarks(
    std::string_view sql,
    const std::vector<TBoundParam>& boundParams) {
    std::string body;
    body.reserve(sql.size());
    size_t questionMarkCount = 0;
    bool inQuote = false;
    size_t braceDepth = 0;
    std::set<SQLUSMALLINT> paramIndices;

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
        if (braceDepth == 0) {
            if (IsParamMark(sql, i)) {
                const auto index = static_cast<SQLUSMALLINT>(++questionMarkCount);
                paramIndices.insert(index);
                body += "$p";
                body += std::to_string(index);
                continue;
            }
            SQLUSMALLINT index = 0;
            if (TryParseDollarParam(sql, i, index)) {
                paramIndices.insert(index);
            }
        }
        body.push_back(ch);
    }

    if (paramIndices.empty()) {
        return {.Sql = std::string(sql)};
    }
    if (questionMarkCount > 0 && questionMarkCount != boundParams.size()) {
        return {.Success = false, .SqlState = "07002", .Message = "COUNT field incorrect"};
    }

    std::string declares;
    for (const SQLUSMALLINT index : paramIndices) {
        const std::string declare = "DECLARE $p" + std::to_string(index) + " AS";
        if (sql.find(declare) != std::string_view::npos) {
            continue;
        }
        const auto bound = std::ranges::find(boundParams, index, &TBoundParam::ParamNumber);
        if (bound == boundParams.end()) {
            return {.Success = false, .SqlState = "07002", .Message = "COUNT field incorrect"};
        }
        declares += declare + " " + FormatYqlParamDeclareType(bound->ParameterType) + ";\n";
    }
    if (declares.empty()) {
        return {.Sql = body};
    }
    return {.Sql = declares + body};
}

} // namespace NYdb::NOdbc
