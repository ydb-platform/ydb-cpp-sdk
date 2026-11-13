#include "param_rewrite.h"

#include "sql_type_map.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <stdexcept>

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
    static constexpr auto MaxParameters = std::numeric_limits<SQLSMALLINT>::max();
    std::string_view Sql_;
    std::string* Output_;
    bool RewriteEscapes_;
    bool AnsiQuotes_ = Sql_.substr(0, SkipSqlTrivia(0, Sql_.size())).find("--!ansi_lexer")
        != std::string_view::npos;

    size_t QuestionCount = 0;
    SQLSMALLINT MaxDollarIndex = 0;
    std::vector<SQLUSMALLINT> Indices;

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

    size_t SkipQuoted(size_t pos, size_t end, bool backslashEscapes = true) const {
        const char quote = Sql_[pos++];
        backslashEscapes = backslashEscapes && (!AnsiQuotes_ || quote == '`');
        while (pos < end) {
            const char ch = Sql_[pos++];
            if (backslashEscapes && ch == '\\' && pos < end) {
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

    size_t FindClose(
        size_t open,
        size_t end,
        char left,
        char right,
        bool backslashEscapes = true) const {
        size_t depth = 1;
        for (size_t pos = open + 1; pos < end;) {
            const size_t commentEnd = SkipComment(pos, end);
            if (commentEnd != pos) {
                pos = commentEnd;
                continue;
            }
            if (Sql_[pos] == '\'' || Sql_[pos] == '"' || Sql_[pos] == '`') {
                pos = SkipQuoted(pos, end, backslashEscapes);
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

    bool ReadQuoted(
        size_t& pos,
        size_t end,
        size_t& valueBegin,
        size_t& valueEnd,
        bool backslashEscapes = true) const {
        pos = SkipTrivia(pos, end);
        if (pos >= end || Sql_[pos] != '\'') {
            return false;
        }
        valueBegin = pos + 1;
        pos = SkipQuoted(pos, end, backslashEscapes);
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
        size_t inner = SkipTrivia(pos + 1, end);
        const bool outputCall = inner + 1 < end && Sql_[inner] == '?' && Sql_[inner + 1] == '=';
        if (outputCall) {
            inner += 2;
        }
        const std::string_view keyword = ReadIdent(inner, end);
        const bool escapeClause = EqualNoCase(keyword, "escape");
        const size_t close = FindClose(pos, end, '{', '}', !escapeClause);
        if (close == std::string_view::npos || inner > close) {
            return false;
        }
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
            || !ReadQuoted(inner, close, valueBegin, valueEnd, !escapeClause)
            || SkipTrivia(inner, close) != close) {
            return false;
        }
        if (EqualNoCase(keyword, "escape")) {
            Append(" ESCAPE '");
            AppendDecoded(valueBegin, valueEnd);
            Append('\'');
        } else {
            Append("CAST('");
            if (EqualNoCase(keyword, "t")) {
                Append("1970-01-01T");
            }
            AppendDecoded(valueBegin, valueEnd, !EqualNoCase(keyword, "d"));
            Append(EqualNoCase(keyword, "d") ? "'s AS Date)"
                : EqualNoCase(keyword, "t") ? "'s AS Datetime)" : "'s AS Timestamp)");
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
            if (value > MaxParameters) {
                throw std::length_error("Too many parameters (maximum 32767)");
            }
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
                if (++QuestionCount > MaxParameters) {
                    throw std::length_error("Too many parameters (maximum 32767)");
                }
                const auto index = static_cast<SQLUSMALLINT>(QuestionCount);
                if (Output_) {
                    Indices.push_back(index);
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
                    Indices.push_back(index);
                }
            }
            Append(Sql_[pos++]);
        }
    }

};

std::vector<std::optional<bool>> ReadDeclarations(std::string_view sql) {
    std::vector<std::optional<bool>> result;
    const TSqlScanner scanner{sql, nullptr, false};
    for (size_t pos = scanner.SkipSqlTrivia(0, sql.size()); pos < sql.size();) {
        const bool declare = StartsWithKeyword(sql, pos, "DECLARE");
        if (!declare && !StartsWithKeyword(sql, pos, "PRAGMA")) break;
        const size_t end = scanner.FindStatementEnd(pos, sql.size());
        if (end == std::string_view::npos) break;
        size_t token = scanner.SkipSqlTrivia(pos + (declare ? 7 : 6), end);
        SQLUSMALLINT number = 0;
        if (declare && scanner.ReadDollar(token, end, number)) {
            const std::string name = "$p" + std::to_string(number);
            if (sql.substr(token, name.size()) == name) {
                token = scanner.SkipSqlTrivia(token + name.size(), end);
                if (StartsWithKeyword(sql, token, "AS")) {
                    const auto type = TrimTrailingSqlTrivia(sql.substr(token + 2, end - token - 2));
                    if (result.size() <= number) result.resize(number + 1);
                    if (!result[number]) result[number] = !type.empty() && type.back() == '?';
                }
            }
        }
        pos = scanner.SkipSqlTrivia(end + 1, sql.size());
    }
    return result;
}

} // namespace

TPreparedQuery::TPreparedQuery(std::string sql)
    : Sql_(std::move(sql))
    , ParameterCount_(CountOdbcParams(Sql_))
    , StatementOffset_(Sql_.size() - GetSqlStatement(Sql_).size())
    , Declarations_(ReadDeclarations(Sql_)) {
    const TSqlScanner scanner{Sql_, nullptr, false};
    const auto end = scanner.FindStatementEnd(StatementOffset_, Sql_.size());
    Multiple_ = end != std::string_view::npos
        && scanner.SkipSqlTrivia(end + 1, Sql_.size()) < Sql_.size();
}

std::optional<bool> TPreparedQuery::DeclaredOptionality(SQLUSMALLINT number) const {
    return number < Declarations_.size() ? Declarations_[number] : std::nullopt;
}

bool TPreparedQuery::StartsWith(std::initializer_list<std::string_view> keywords) const {
    return std::ranges::any_of(keywords, [&](auto keyword) {
        return StartsWithKeyword(Sql_, StatementOffset_, keyword);
    });
}

TParamRewriteResult TPreparedQuery::Bind(const std::vector<TBoundParam>& params, bool escapes) {
    auto& cached = Bodies_[escapes];
    if (!cached) {
        std::string body;
        body.reserve(Sql_.size());
        TSqlScanner scanner{Sql_, &body, escapes};
        scanner.Scan(0, Sql_.size(), true);
        std::ranges::sort(scanner.Indices);
        scanner.Indices.erase(std::unique(scanner.Indices.begin(), scanner.Indices.end()), scanner.Indices.end());
        cached.emplace(TBody{std::move(body), scanner.QuestionCount, std::move(scanner.Indices)});
    }
    if (cached->QuestionCount && cached->QuestionCount != params.size()) {
        return {.Success = false, .SqlState = "07002", .Message = "COUNT field incorrect: unbound ? placeholders"};
    }
    const bool ordered = std::ranges::is_sorted(params, {}, &TBoundParam::ParamNumber);
    auto bound = params.begin();
    std::string declarations;
    for (const auto number : cached->Indices) {
        if (DeclaredOptionality(number).has_value()) continue;
        if (ordered) {
            while (bound != params.end() && bound->ParamNumber < number) ++bound;
        } else {
            bound = std::ranges::find(params, number, &TBoundParam::ParamNumber);
        }
        if (bound == params.end() || bound->ParamNumber != number) {
            return {.Success = false, .SqlState = "07002", .Message = "COUNT field incorrect"};
        }
        const auto type = ResolveParamType(*bound);
        if (!type) {
            return {.Success = false, .SqlState = "07006", .Message = "Restricted data type attribute violation"};
        }
        declarations += "DECLARE $p" + std::to_string(number) + " AS " + type->YqlType
            + (BoundParamIsNull(*bound) ? "?;\n" : ";\n");
    }
    return {.Sql = declarations.empty() ? cached->Sql : AddSqlPreamble(cached->Sql, declarations)};
}

std::string AddSqlPreamble(std::string_view sql, std::string_view preamble) {
    // Lexer directives must precede generated statements.
    const TSqlScanner scanner{sql, nullptr, false};
    const size_t pos = scanner.SkipSqlTrivia(0, sql.size());
    std::string result;
    result.reserve(sql.size() + preamble.size());
    result.append(sql.substr(0, pos));
    result.append(preamble);
    result.append(sql.substr(pos));
    return result;
}

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
    return TPreparedQuery(std::string(sql)).Bind(boundParams, rewriteEscapes);
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

std::optional<std::string> BuildParamMetadataQuery(
    std::string_view input, bool rewriteEscapes, SQLUSMALLINT paramNumber, SQLSMALLINT paramCount) {
    const std::string sql = rewriteEscapes ? RewriteOdbcEscapes(std::string(input)) : std::string(input);
    if (HasMultipleSqlStatements(sql)) {
        return std::nullopt;
    }
    const auto count = CountOdbcParams(sql);
    if (count != paramCount || paramNumber < 1 || paramNumber > count) {
        return std::nullopt;
    }
    const TSqlScanner scanner{sql, nullptr, false};
    struct TToken {
        std::string_view Text;
        SQLUSMALLINT Parameter = 0;
    };
    std::vector<TToken> tokens;
    SQLUSMALLINT question = 0;
    const size_t bodyOffset = sql.size() - GetSqlStatement(sql).size();
    const size_t trivia = scanner.SkipSqlTrivia(0, sql.size());
    const auto prologue = TrimTrailingSqlTrivia(std::string_view(sql).substr(trivia, bodyOffset - trivia));
    if (!prologue.empty() && (!scanner.AnsiQuotes_ || prologue != "PRAGMA UnicodeLiterals;")) {
        return std::nullopt;
    }
    for (size_t pos = bodyOffset; (pos = scanner.SkipSqlTrivia(pos, sql.size())) < sql.size();) {
        const size_t begin = pos;
        SQLUSMALLINT parameter = 0;
        const unsigned char ch = sql[pos++];
        if (ch == '\'' || ch == '"' || ch == '`') {
            pos = scanner.SkipQuoted(begin, sql.size());
            if (pos == std::string_view::npos) {
                return std::nullopt;
            }
        } else if (ch == '?' && scanner.IsQuestionMark(begin)) {
            parameter = ++question;
        } else if (std::isalnum(ch) || ch == '_' || ch == '$') {
            while (pos < sql.size() && (std::isalnum(static_cast<unsigned char>(sql[pos]))
                    || sql[pos] == '_' || (std::isdigit(ch) && sql[pos] == '.')
                    || (std::isdigit(ch) && (sql[pos - 1] == 'e' || sql[pos - 1] == 'E')
                        && (sql[pos] == '+' || sql[pos] == '-')))) {
                ++pos;
            }
            scanner.ReadDollar(begin, pos, parameter);
        } else if (pos < sql.size() && (ch == '<' || ch == '>' || ch == '!')
                   && (sql[pos] == '=' || (ch == '<' && sql[pos] == '>'))) {
            ++pos;
        }
        tokens.push_back({std::string_view(sql).substr(begin, pos - begin), parameter});
    }
    const auto is = [&](size_t i, std::string_view text) {
        return i < tokens.size() && EqualNoCase(tokens[i].Text, text);
    };
    const auto identifier = [&](size_t i) {
        return i < tokens.size() && !tokens[i].Parameter
            && (std::isalpha(static_cast<unsigned char>(tokens[i].Text.front()))
                || tokens[i].Text.front() == '_' || tokens[i].Text.front() == '`');
    };
    const auto nameEnd = [&](size_t i) {
        if (!identifier(i)) {
            return i;
        }
        while (is(i + 1, ".") && identifier(i + 2)) {
            i += 2;
        }
        return i + 1;
    };
    const auto span = [&](size_t begin, size_t end) {
        return std::string(tokens[begin].Text.data(),
            tokens[end - 1].Text.data() + tokens[end - 1].Text.size());
    };
    std::vector<std::string> sources(static_cast<size_t>(count));
    const auto assign = [&](SQLUSMALLINT parameter, std::string expression) {
        if (parameter > 0 && parameter <= sources.size() && !expression.empty()) {
            auto& source = sources[parameter - 1];
            source = source.empty() || source == expression ? std::move(expression) : "NULL";
        }
    };
    constexpr size_t body = 0;
    if (!is(body, "SELECT") && !is(body, "INSERT") && !is(body, "UPSERT") && !is(body, "REPLACE")
        && !is(body, "UPDATE") && !is(body, "DELETE")) {
        return std::nullopt;
    }
    std::string table;
    size_t tableEnd = 0;
    int depth = 0;
    for (size_t i = body; i < tokens.size(); ++i) {
        if (is(i, "(")) ++depth;
        if (is(i, ")")) --depth;
        if (depth == 0 && (is(i, "FROM") || is(i, "INTO") || (i == body && is(i, "UPDATE")))) {
            const auto end = nameEnd(i + 1);
            if (end == i + 1 || !table.empty()) {
                return std::nullopt;
            }
            table = span(i + 1, end);
            tableEnd = end;
            if (is(body, "SELECT") || is(body, "DELETE")) {
                const auto clause = [&](size_t i) {
                    return i == tokens.size() || is(i, ";") || is(i, "WHERE") || is(i, "ORDER")
                        || is(i, "GROUP") || is(i, "HAVING") || is(i, "LIMIT") || is(i, "OFFSET")
                        || is(i, "UNION") || is(i, "WINDOW");
                };
                size_t tail = end;
                if (is(tail, "AS")) {
                    if (!identifier(++tail)) return std::nullopt;
                    table += " AS " + std::string(tokens[tail++].Text);
                } else if (identifier(tail) && !clause(tail)) {
                    table += " AS " + std::string(tokens[tail++].Text);
                }
                if (!clause(tail)) return std::nullopt;
            }
        }
        if (is(i, "JOIN") || (is(i, "SELECT") && i != body)) {
            return std::nullopt;
        }
    }
    if (depth != 0) return std::nullopt;
    if ((is(body, "INSERT") || is(body, "UPSERT") || is(body, "REPLACE")) && is(tableEnd, "(")) {
        std::vector<std::string> columns;
        size_t pos = tableEnd + 1;
        while (identifier(pos)) {
            columns.emplace_back(tokens[pos++].Text);
            if (!is(pos, ",")) break;
            ++pos;
        }
        if (!columns.empty() && is(pos, ")") && is(pos + 1, "VALUES")) {
            for (pos += 2; is(pos, "("); ++pos) {
                size_t start = ++pos, column = 0;
                int nested = 0;
                for (; pos < tokens.size(); ++pos) {
                    if (nested == 0 && (is(pos, ",") || is(pos, ")"))) {
                        if (column < columns.size() && pos == start + 1) {
                            assign(tokens[start].Parameter, columns[column]);
                        }
                        ++column;
                        start = pos + 1;
                        if (is(pos, ")")) break;
                    }
                    if (is(pos, "(")) ++nested;
                    if (is(pos, ")")) --nested;
                }
                if (!is(pos + 1, ",")) break;
                ++pos;
            }
        }
    }
    const auto boundary = [&](size_t i) {
        return i >= tokens.size() || is(i, "(") || is(i, ")") || is(i, ",") || is(i, ";")
            || is(i, "WHERE") || is(i, "SET") || is(i, "AND") || is(i, "OR")
            || is(i, "WHEN") || is(i, "THEN") || is(i, "ORDER") || is(i, "LIMIT")
            || is(i, "SELECT") || is(i, "AS");
    };
    const auto comparison = [&](size_t i) {
        return is(i, "=") || is(i, "<>") || is(i, "!=") || is(i, "<") || is(i, "<=") || is(i, ">") || is(i, ">=");
    };
    for (size_t i = body; i < tokens.size(); ++i) {
        size_t end = i + 1;
        std::string expression;
        const auto text = tokens[i].Text;
        if (std::isdigit(static_cast<unsigned char>(text.front())) || is(i, "TRUE") || is(i, "FALSE")) {
            expression = "Just(" + std::string(text) + ")";
        } else if ((is(i, "-") || is(i, "+")) && i + 1 < tokens.size()
                   && std::isdigit(static_cast<unsigned char>(tokens[i + 1].Text.front()))) {
            expression = "Just(" + span(i, ++end) + ")";
        } else if (!table.empty() && identifier(i)) {
            end = nameEnd(i);
            expression = span(i, end);
        }
        if (expression.empty()) continue;
        // A floating literal fixes the arithmetic result type without narrowing integer inputs.
        const auto operation = [&](size_t op) {
            return comparison(op) || (text.find_first_of(".eE") != std::string_view::npos
                && std::isdigit(static_cast<unsigned char>(text.front()))
                && (is(op, "+") || is(op, "-") || is(op, "*") || is(op, "/")));
        };
        if ((i == body || boundary(i - 1)) && operation(end) && end + 1 < tokens.size()
            && tokens[end + 1].Parameter && boundary(end + 2)) {
            assign(tokens[end + 1].Parameter, expression);
        }
        if (i >= body + 3 && (i == body + 3 || boundary(i - 3))
            && tokens[i - 2].Parameter && operation(i - 1) && !is(end, "(") && boundary(end)) {
            assign(tokens[i - 2].Parameter, expression);
        }
    }
    if (sources[paramNumber - 1].empty() || sources[paramNumber - 1] == "NULL") {
        return std::nullopt;
    }
    std::string query = "SELECT ";
    for (size_t i = 0; i < sources.size(); ++i) {
        if (i) query += ", ";
        query += (sources[i].empty() ? "NULL" : sources[i]) + " AS __p" + std::to_string(i + 1);
    }
    return (scanner.AnsiQuotes_ ? sql.substr(0, bodyOffset) : std::string{})
        + query + (table.empty() ? "" : " FROM " + table) + " LIMIT 0";
}

} // namespace NYdb::NOdbc
