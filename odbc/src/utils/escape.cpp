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
            if (Sql_[pos++] != quote) {
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
        if (sql.find(prefix) != std::string_view::npos) {
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
        declarations += prefix + " " + type->YqlType + "?;\n";
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
    size_t pos = 0;
    while (pos < sql.size()) {
        if (std::isspace(static_cast<unsigned char>(sql[pos]))) {
            ++pos;
        } else if (pos + 1 < sql.size() && sql[pos] == '-' && sql[pos + 1] == '-') {
            const size_t newline = sql.find('\n', pos + 2);
            pos = newline == std::string_view::npos ? sql.size() : newline + 1;
        } else if (pos + 1 < sql.size() && sql[pos] == '/' && sql[pos + 1] == '*') {
            const size_t close = sql.find("*/", pos + 2);
            pos = close == std::string_view::npos ? sql.size() : close + 2;
        } else {
            break;
        }
    }
    return std::ranges::any_of(keywords, [&](std::string_view keyword) {
        return sql.size() - pos >= keyword.size()
            && EqualNoCase(sql.substr(pos, keyword.size()), keyword);
    });
}

} // namespace NYdb::NOdbc
