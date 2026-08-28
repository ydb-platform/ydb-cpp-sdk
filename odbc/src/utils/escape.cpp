#include "escape.h"

#include "sql_type_map.h"

#include <algorithm>
#include <bitset>
#include <cctype>

namespace NYdb::NOdbc {
namespace {

bool EqualNoCase(std::string_view lhs, std::string_view rhs) {
    return lhs.size() == rhs.size() && std::ranges::equal(lhs, rhs, [](unsigned char lhs, unsigned char rhs) {
        return std::tolower(lhs) == std::tolower(rhs);
    });
}

bool StartsWithKeyword(std::string_view sql, size_t pos, std::string_view keyword) {
    if (pos > sql.size()
        || (pos > 0 && (std::isalnum(static_cast<unsigned char>(sql[pos - 1]))
            || sql[pos - 1] == '_'))
        || sql.size() - pos < keyword.size()
        || !EqualNoCase(sql.substr(pos, keyword.size()), keyword)) {
        return false;
    }
    const size_t end = pos + keyword.size();
    return end == sql.size()
        || (!std::isalnum(static_cast<unsigned char>(sql[end])) && sql[end] != '_');
}

struct TSqlScanner {
    std::string_view Sql_;
    std::string* Output_;
    bool RewriteEscapes_;

    size_t QuestionCount = 0;
    SQLSMALLINT MaxDollarIndex = 0;
    std::bitset<1U << 16U> Indices;

    void Append(std::string_view text) {
        if (Output_) {
            Output_->append(text);
        }
    }

    void Append(char ch) {
        if (Output_) {
            Output_->push_back(ch);
        }
    }

    size_t SkipTrivia(size_t pos, size_t end) const {
        while (pos < end && std::isspace(static_cast<unsigned char>(Sql_[pos]))) {
            ++pos;
        }
        return pos;
    }

    size_t SkipQuoted(size_t pos, size_t end) const {
        const char quote = Sql_[pos++];
        while (pos < end) {
            const char ch = Sql_[pos++];
            if (ch == '\\' && pos < end) {
                ++pos;
                continue;
            }
            if (ch != quote) {
                continue;
            }
            if (pos < end && Sql_[pos] == quote) {
                ++pos;
            } else {
                return pos;
            }
        }
        return std::string_view::npos;
    }

    size_t SkipComment(size_t pos, size_t end) const {
        if (pos + 1 >= end) {
            return pos;
        }
        if (Sql_[pos] == '-' && Sql_[pos + 1] == '-') {
            const size_t newline = Sql_.find('\n', pos + 2);
            return newline == std::string_view::npos || newline >= end ? end : newline + 1;
        }
        if (Sql_[pos] == '/' && Sql_[pos + 1] == '*') {
            const size_t close = Sql_.find("*/", pos + 2);
            return close == std::string_view::npos || close + 2 > end ? end : close + 2;
        }
        return pos;
    }

    size_t SkipSqlTrivia(size_t pos, size_t end) const {
        while (pos < end) {
            pos = SkipTrivia(pos, end);
            const size_t commentEnd = SkipComment(pos, end);
            if (commentEnd == pos) {
                break;
            }
            pos = commentEnd;
        }
        return pos;
    }

    size_t FindStatementEnd(size_t pos, size_t end) const {
        while (pos < end) {
            const size_t commentEnd = SkipComment(pos, end);
            if (commentEnd != pos) {
                pos = commentEnd;
            } else if (Sql_[pos] == '\'' || Sql_[pos] == '"' || Sql_[pos] == '`') {
                pos = std::min(SkipQuoted(pos, end), end);
            } else if (Sql_[pos] == ';') {
                return pos;
            } else {
                ++pos;
            }
        }
        return std::string_view::npos;
    }

    size_t FindDefineEnd(size_t pos, size_t end) const {
        while (pos < end) {
            const size_t commentEnd = SkipComment(pos, end);
            if (commentEnd != pos) {
                pos = commentEnd;
                continue;
            }
            if (Sql_[pos] == '\'' || Sql_[pos] == '"' || Sql_[pos] == '`') {
                pos = std::min(SkipQuoted(pos, end), end);
                continue;
            }
            if (StartsWithKeyword(Sql_, pos, "END")) {
                const size_t define = SkipSqlTrivia(pos + 3, end);
                if (StartsWithKeyword(Sql_, define, "DEFINE")) {
                    const size_t semicolon = SkipSqlTrivia(define + 6, end);
                    if (semicolon < end && Sql_[semicolon] == ';') {
                        return semicolon;
                    }
                }
            }
            ++pos;
        }
        return std::string_view::npos;
    }

    bool IsNamedExpressionAssignment(size_t pos, size_t end) const {
        if (pos >= end || Sql_[pos++] != '$' || pos >= end
            || (!std::isalpha(static_cast<unsigned char>(Sql_[pos])) && Sql_[pos] != '_')) {
            return false;
        }
        while (++pos < end
            && (std::isalnum(static_cast<unsigned char>(Sql_[pos])) || Sql_[pos] == '_')) {
        }
        pos = SkipSqlTrivia(pos, end);
        return pos < end && Sql_[pos] == '=';
    }

    size_t FindPrologueEnd(size_t pos, size_t end) const {
        if (StartsWithKeyword(Sql_, pos, "DEFINE")) {
            return FindDefineEnd(pos + 6, end);
        }
        if (StartsWithKeyword(Sql_, pos, "DECLARE")
            || StartsWithKeyword(Sql_, pos, "PRAGMA")
            || IsNamedExpressionAssignment(pos, end)) {
            return FindStatementEnd(pos, end);
        }
        return std::string_view::npos;
    }

    size_t FindClose(size_t open, size_t end, char left, char right) const {
        size_t depth = 1;
        for (size_t pos = open + 1; pos < end;) {
            const size_t commentEnd = SkipComment(pos, end);
            if (commentEnd != pos) {
                pos = commentEnd;
                continue;
            }
            if (Sql_[pos] == '\'' || Sql_[pos] == '"' || Sql_[pos] == '`') {
                pos = SkipQuoted(pos, end);
                continue;
            }
            if (Sql_[pos] == left) {
                ++depth;
            } else if (Sql_[pos] == right && --depth == 0) {
                return pos;
            }
            ++pos;
        }
        return std::string_view::npos;
    }

    std::string_view ReadIdent(size_t& pos, size_t end) const {
        pos = SkipTrivia(pos, end);
        const size_t start = pos;
        while (pos < end && (std::isalpha(static_cast<unsigned char>(Sql_[pos])) || Sql_[pos] == '_')) {
            ++pos;
        }
        return Sql_.substr(start, pos - start);
    }

    bool ReadQuoted(size_t& pos, size_t end, size_t& valueBegin, size_t& valueEnd) const {
        pos = SkipTrivia(pos, end);
        if (pos >= end || Sql_[pos] != '\'') {
            return false;
        }
        valueBegin = pos + 1;
        pos = SkipQuoted(pos, end);
        if (pos != std::string_view::npos) {
            valueEnd = pos - 1;
            return true;
        }
        return false;
    }

    void AppendDecoded(size_t begin, size_t end, bool timestamp = false) {
        bool hasZulu = false;
        bool changedSpace = false;
        for (size_t pos = begin; pos < end; ++pos) {
            char ch = Sql_[pos];
            if (ch == '\'' && pos + 1 < end && Sql_[pos + 1] == '\'') {
                ++pos;
            }
            if (timestamp && ch == ' ' && !changedSpace) {
                ch = 'T';
                changedSpace = true;
            }
            hasZulu = hasZulu || ch == 'Z';
            Append(ch);
        }
        if (timestamp && !hasZulu) {
            Append('Z');
        }
    }

    bool RewriteBrace(size_t& pos, size_t end, bool parameters) {
        const size_t close = FindClose(pos, end, '{', '}');
        if (close == std::string_view::npos) {
            return false;
        }
        size_t inner = SkipTrivia(pos + 1, close);
        const bool outputCall = inner + 1 < close && Sql_[inner] == '?' && Sql_[inner + 1] == '=';
        if (outputCall) {
            inner += 2;
        }
        const std::string_view keyword = ReadIdent(inner, close);
        if (outputCall && !EqualNoCase(keyword, "call")) {
            return false;
        }
        const bool body = EqualNoCase(keyword, "fn") || EqualNoCase(keyword, "oj")
            || EqualNoCase(keyword, "call");
        if (body) {
            inner = SkipTrivia(inner, close);
            if (EqualNoCase(keyword, "call")) {
                Append("CALL ");
            }
            Scan(inner, close, parameters);
            pos = close + 1;
            return true;
        }
        size_t valueBegin = 0, valueEnd = 0;
        if ((!EqualNoCase(keyword, "d") && !EqualNoCase(keyword, "t")
             && !EqualNoCase(keyword, "ts") && !EqualNoCase(keyword, "escape"))
            || !ReadQuoted(inner, close, valueBegin, valueEnd) || SkipTrivia(inner, close) != close) {
            return false;
        }
        if (EqualNoCase(keyword, "escape")) {
            Append(" ESCAPE '");
            AppendDecoded(valueBegin, valueEnd);
            Append('\'');
        } else {
            Append("CAST('");
            AppendDecoded(valueBegin, valueEnd, EqualNoCase(keyword, "ts"));
            Append(EqualNoCase(keyword, "d") ? "' AS Date)"
                : EqualNoCase(keyword, "t") ? "' AS Time)" : "' AS Datetime)");
        }
        pos = close + 1;
        return true;
    }

    bool RewriteConvert(size_t& pos, size_t end, bool parameters) {
        constexpr std::string_view Token = "CONVERT";
        if (pos + Token.size() > end || !EqualNoCase(Sql_.substr(pos, Token.size()), Token)) {
            return false;
        }
        size_t open = SkipTrivia(pos + Token.size(), end);
        if (open >= end || Sql_[open] != '(') {
            return false;
        }
        const size_t close = FindClose(open, end, '(', ')');
        if (close == std::string_view::npos) {
            return false;
        }
        size_t comma = open + 1;
        size_t depth = 1;
        for (; comma < close;) {
            const size_t commentEnd = SkipComment(comma, close);
            if (commentEnd != comma) {
                comma = commentEnd;
                continue;
            }
            if (Sql_[comma] == '\'' || Sql_[comma] == '"' || Sql_[comma] == '`') {
                comma = SkipQuoted(comma, close);
                continue;
            }
            if (Sql_[comma] == '(') {
                ++depth;
            } else if (Sql_[comma] == ')') {
                --depth;
            } else if (Sql_[comma] == ',' && depth == 1) {
                break;
            }
            ++comma;
        }
        size_t typePos = comma < close ? comma + 1 : close;
        const std::string_view type = ReadIdent(typePos, close);
        if (comma == close || type.empty() || SkipTrivia(typePos, close) != close) {
            return false;
        }
        Append("CAST(");
        Scan(open + 1, comma, parameters);
        Append(" AS ");
        Append(MapSqlTypeToken(type));
        Append(')');
        pos = close + 1;
        return true;
    }

    bool IsQuestionMark(size_t pos) const {
        const unsigned char previous = pos == 0 ? 0 : Sql_[pos - 1];
        return Sql_[pos] == '?' && (pos == 0 || ((!std::isalnum(previous) && previous != '_') && previous != ')'));
    }

    bool ReadDollar(size_t pos, size_t end, SQLUSMALLINT& index) const {
        if (pos + 2 >= end || Sql_[pos] != '$' || Sql_[pos + 1] != 'p'
            || !std::isdigit(static_cast<unsigned char>(Sql_[pos + 2]))) {
            return false;
        }
        unsigned value = 0;
        for (size_t i = pos + 2; i < end && std::isdigit(static_cast<unsigned char>(Sql_[i])); ++i) {
            value = value * 10 + static_cast<unsigned>(Sql_[i] - '0');
        }
        index = static_cast<SQLUSMALLINT>(value);
        return true;
    }

    void Scan(size_t begin, size_t end, bool parameters) {
        for (size_t pos = begin; pos < end;) {
            const size_t commentEnd = SkipComment(pos, end);
            if (commentEnd != pos) {
                Append(Sql_.substr(pos, commentEnd - pos));
                pos = commentEnd;
                continue;
            }
            if (Sql_[pos] == '\'' || Sql_[pos] == '"' || Sql_[pos] == '`') {
                const size_t next = std::min(SkipQuoted(pos, end), end);
                Append(Sql_.substr(pos, next - pos));
                pos = next;
                continue;
            }
            if (Sql_[pos] == '{') {
                if (RewriteEscapes_ && RewriteBrace(pos, end, parameters)) {
                    continue;
                }
                const size_t close = FindClose(pos, end, '{', '}');
                const size_t next = close == std::string_view::npos ? end : close + 1;
                Append(Sql_.substr(pos, next - pos));
                pos = next;
                continue;
            }
            if (RewriteEscapes_ && RewriteConvert(pos, end, parameters)) {
                continue;
            }
            if (parameters && IsQuestionMark(pos)) {
                const SQLUSMALLINT index = static_cast<SQLUSMALLINT>(++QuestionCount);
                if (Output_) {
                    Indices.set(index);
                    Append("$p");
                    Append(std::to_string(index));
                }
                ++pos;
                continue;
            }
            SQLUSMALLINT index = 0;
            if (parameters && ReadDollar(pos, end, index)) {
                MaxDollarIndex = std::max(MaxDollarIndex, static_cast<SQLSMALLINT>(index));
                if (Output_) {
                    Indices.set(index);
                }
            }
            Append(Sql_[pos++]);
        }
    }

};

TParamRewriteResult RewriteSql(
    std::string_view sql,
    const std::vector<TBoundParam>& boundParams,
    bool escapes) {
    std::string body;
    body.reserve(sql.size());
    TSqlScanner scanner{sql, &body, escapes};
    scanner.Scan(0, sql.size(), true);
    if (scanner.Indices.none()) {
        return {.Sql = std::move(body)};
    }
    if (scanner.QuestionCount > 0 && scanner.QuestionCount != boundParams.size()) {
        return {.Success = false, .SqlState = "07002", .Message = "COUNT field incorrect"};
    }
    std::string declarations;
    for (size_t rawIndex = 0; rawIndex < scanner.Indices.size(); ++rawIndex) {
        if (!scanner.Indices.test(rawIndex)) {
            continue;
        }
        const auto index = static_cast<SQLUSMALLINT>(rawIndex);
        const std::string prefix = "DECLARE $p" + std::to_string(index) + " AS";
        if (GetDeclaredParamOptionality(sql, index).has_value()) {
            continue;
        }
        const auto bound = std::ranges::find(boundParams, index, &TBoundParam::ParamNumber);
        if (bound == boundParams.end()) {
            return {.Success = false, .SqlState = "07002", .Message = "COUNT field incorrect"};
        }
        const auto type = ResolveParamType(*bound);
        if (!type) {
            return {.Success = false, .SqlState = "07006", .Message = "Restricted data type attribute violation"};
        }
        declarations += prefix + " " + type->YqlType
            + (BoundParamIsNull(*bound) ? "?;\n" : ";\n");
    }
    return {.Sql = declarations.empty() ? std::move(body) : declarations + body};
}

} // namespace

std::string RewriteOdbcEscapes(const std::string& sql) {
    std::string result;
    result.reserve(sql.size());
    TSqlScanner scanner{sql, &result, true};
    scanner.Scan(0, sql.size(), false);
    return result;
}

TParamRewriteResult RewriteOdbcSql(
    std::string_view sql,
    const std::vector<TBoundParam>& boundParams,
    bool rewriteEscapes) {
    return RewriteSql(sql, boundParams, rewriteEscapes);
}

std::optional<bool> GetDeclaredParamOptionality(
    std::string_view sql,
    SQLUSMALLINT paramNumber)
{
    const std::string param = "$p" + std::to_string(paramNumber);
    const TSqlScanner scanner{sql, nullptr, false};
    for (size_t pos = scanner.SkipSqlTrivia(0, sql.size()); pos < sql.size();) {
        const bool isDeclare = StartsWithKeyword(sql, pos, "DECLARE");
        if (!isDeclare && !StartsWithKeyword(sql, pos, "PRAGMA")) {
            break;
        }
        const size_t semicolon = scanner.FindStatementEnd(pos, sql.size());
        if (semicolon == std::string_view::npos) {
            break;
        }
        size_t token = scanner.SkipSqlTrivia(pos + (isDeclare ? 7 : 6), semicolon);
        if (isDeclare && semicolon - token >= param.size()
            && sql.substr(token, param.size()) == param
            && (token + param.size() == semicolon
                || (!std::isalnum(static_cast<unsigned char>(sql[token + param.size()]))
                    && sql[token + param.size()] != '_'))) {
            token = scanner.SkipSqlTrivia(token + param.size(), semicolon);
            if (StartsWithKeyword(sql, token, "AS")) {
                const std::string_view type = TrimTrailingSqlTrivia(
                    sql.substr(token + 2, semicolon - token - 2));
                return !type.empty() && type.back() == '?';
            }
        }
        pos = scanner.SkipSqlTrivia(semicolon + 1, sql.size());
    }
    return std::nullopt;
}

std::string_view TrimTrailingSqlTrivia(std::string_view sql) {
    const TSqlScanner scanner{sql, nullptr, false};
    size_t codeEnd = 0;
    for (size_t pos = 0; pos < sql.size();) {
        const size_t commentEnd = scanner.SkipComment(pos, sql.size());
        if (commentEnd != pos) {
            if (sql[pos] == '/' && sql.find("*/", pos + 2) == std::string_view::npos) {
                return sql;
            }
            pos = commentEnd;
        } else if (std::isspace(static_cast<unsigned char>(sql[pos]))) {
            ++pos;
        } else if (sql[pos] == '\'' || sql[pos] == '"' || sql[pos] == '`') {
            pos = std::min(scanner.SkipQuoted(pos, sql.size()), sql.size());
            codeEnd = pos;
        } else {
            codeEnd = ++pos;
        }
    }
    return sql.substr(0, codeEnd);
}

std::string_view GetSqlStatement(std::string_view sql) {
    const TSqlScanner scanner{sql, nullptr, false};
    size_t statement = scanner.SkipSqlTrivia(0, sql.size());
    while (statement < sql.size()) {
        const size_t prologueEnd = scanner.FindPrologueEnd(statement, sql.size());
        if (prologueEnd == std::string_view::npos) {
            break;
        }
        const size_t next = scanner.SkipSqlTrivia(prologueEnd + 1, sql.size());
        if (next == sql.size()) {
            break;
        }
        statement = next;
    }
    return sql.substr(statement);
}

bool HasMultipleSqlStatements(std::string_view sql) {
    const TSqlScanner scanner{sql, nullptr, false};
    const std::string_view statement = GetSqlStatement(sql);
    const size_t statementBegin = sql.size() - statement.size();
    const size_t statementEnd = scanner.FindStatementEnd(statementBegin, sql.size());
    return statementEnd != std::string_view::npos
        && scanner.SkipSqlTrivia(statementEnd + 1, sql.size()) < sql.size();
}

SQLSMALLINT CountOdbcParams(std::string_view sql) {
    TSqlScanner scanner{sql, nullptr, false};
    scanner.Scan(0, sql.size(), true);
    return scanner.QuestionCount > 0
        ? static_cast<SQLSMALLINT>(scanner.QuestionCount)
        : scanner.MaxDollarIndex;
}

bool StartsWithSqlStatement(
    std::string_view sql,
    std::initializer_list<std::string_view> keywords) {
    sql = GetSqlStatement(sql);
    return std::ranges::any_of(keywords, [&](std::string_view keyword) {
        return StartsWithKeyword(sql, 0, keyword);
    });
}

} // namespace NYdb::NOdbc
