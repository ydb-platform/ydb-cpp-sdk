#include "escape.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace NYdb::NOdbc {
namespace {

bool EqualNoCase(std::string_view lhs, std::string_view rhs) {
    return lhs.size() == rhs.size() &&
        std::equal(lhs.begin(), lhs.end(), rhs.begin(), [](char leftCh, char rightCh) {
            return std::tolower(static_cast<unsigned char>(leftCh)) ==
                std::tolower(static_cast<unsigned char>(rightCh));
        });
}

void SkipLeadingWhitespace(std::string_view sql, size_t& cursor) {
    const auto strEnd = sql.end();
    const auto firstNonSpace = std::find_if_not(
        sql.begin() + static_cast<std::ptrdiff_t>(cursor),
        strEnd,
        [](unsigned char byte) {
            return std::isspace(byte) != 0;
        });
    cursor = static_cast<size_t>(firstNonSpace - sql.begin());
}

bool ReadIdent(std::string_view sql, size_t& cursor, std::string_view* outIdent) {
    SkipLeadingWhitespace(sql, cursor);
    const size_t identStart = cursor;
    const auto afterIdent = std::find_if_not(
        sql.begin() + static_cast<std::ptrdiff_t>(cursor),
        sql.end(),
        [](unsigned char byte) {
            return std::isalpha(byte) != 0 || byte == '_';
        });
    cursor = static_cast<size_t>(afterIdent - sql.begin());
    if (cursor == identStart) {
        return false;
    }
    *outIdent = std::string_view(sql.data() + identStart, cursor - identStart);
    return true;
}

bool ParseSingleQuoted(std::string_view sql, size_t& cursor, std::string* outValue) {
    SkipLeadingWhitespace(sql, cursor);
    if (cursor >= sql.size() || sql[cursor] != '\'') {
        return false;
    }
    ++cursor;
    outValue->clear();
    while (cursor < sql.size()) {
        if (sql[cursor] == '\'') {
            if (cursor + 1 < sql.size() && sql[cursor + 1] == '\'') {
                outValue->push_back('\'');
                cursor += 2;
                continue;
            }
            ++cursor;
            return true;
        }
        outValue->push_back(sql[cursor++]);
    }
    return false;
}

size_t FindMatchingCloseBrace(std::string_view sql, size_t openBrace) {
    if (openBrace >= sql.size() || sql[openBrace] != '{') {
        return std::string_view::npos;
    }
    int braceDepth = 1;
    for (size_t idx = openBrace + 1; idx < sql.size(); ++idx) {
        if (sql[idx] == '{') {
            ++braceDepth;
        } else if (sql[idx] == '}') {
            --braceDepth;
            if (braceDepth == 0) {
                return idx;
            }
        }
    }
    return std::string_view::npos;
}

std::string NormalizeOdbcTimestampLiteral(const std::string& raw) {
    std::string normalized = raw;
    const auto firstSpace = std::find(normalized.begin(), normalized.end(), ' ');
    if (firstSpace != normalized.end()) {
        *firstSpace = 'T';
    }
    if (std::find(normalized.begin(), normalized.end(), 'Z') == normalized.end()) {
        normalized.push_back('Z');
    }
    return normalized;
}

std::string ToUpperAscii(std::string_view sv) {
    std::string upper;
    upper.resize(sv.size());
    std::transform(sv.begin(), sv.end(), upper.begin(), [](unsigned char byte) {
        return static_cast<char>(std::toupper(byte));
    });
    return upper;
}

std::string MapSqlTypeToken(std::string_view sqlType) {
    static const std::unordered_map<std::string, std::string> kMap = {
        {"CHAR", "Utf8"},
        {"VARCHAR", "Utf8"},
        {"LONGVARCHAR", "Utf8"},
        {"WCHAR", "Utf8"},
        {"WVARCHAR", "Utf8"},
        {"WLONGVARCHAR", "Utf8"},
        {"BIT", "Bool"},
        {"TINYINT", "Int8"},
        {"SMALLINT", "Int16"},
        {"INTEGER", "Int32"},
        {"BIGINT", "Int64"},
        {"REAL", "Float"},
        {"FLOAT", "Double"},
        {"DOUBLE", "Double"},
        {"DECIMAL", "Decimal(22, 9)"},
        {"NUMERIC", "Decimal(22, 9)"},
        {"BINARY", "String"},
        {"VARBINARY", "String"},
        {"LONGVARBINARY", "String"},
        {"DATE", "Date"},
        {"TIME", "Time"},
        {"TIMESTAMP", "Datetime"},
        {"TYPE_DATE", "Date"},
        {"TYPE_TIME", "Time"},
        {"TYPE_TIMESTAMP", "Datetime"},
    };
    std::string key = ToUpperAscii(sqlType);
    const std::string kSql = "SQL_";
    if (key.size() > kSql.size() && key.compare(0, kSql.size(), kSql) == 0) {
        key.erase(0, kSql.size());
    }
    const auto mapped = kMap.find(key);
    if (mapped != kMap.end()) {
        return mapped->second;
    }
    return key;
}

std::string RewriteOdbcEscapesImpl(std::string_view sql);


enum class OdbcBraceKind {
    OutputProcedureCall, // {?= call ... }
    FnBody,              // {fn ...}
    OjBody,              // {oj ...}
    DateLiteral,         // {d '...'}
    TimeLiteral,         // {t '...'}
    TimestampLiteral,    // {ts '...'}
    ProcedureCall,       // {call ...}
    LikeEscape,          // {escape '...'}
};

struct OdbcBraceParsed {
    OdbcBraceKind Kind;
    std::string_view RecurseTail;
    std::string QuotedValue;
};

std::optional<OdbcBraceParsed> TryParseOutputCallBrace(std::string_view sql, size_t parsePos, size_t closeBrace) {
    if (parsePos + 1 >= sql.size() || sql[parsePos] != '?' || sql[parsePos + 1] != '=') {
        return std::nullopt;
    }
    size_t inner = parsePos + 2;
    SkipLeadingWhitespace(sql, inner);
    std::string_view keyword;
    if (!ReadIdent(sql, inner, &keyword) || !EqualNoCase(keyword, "call")) {
        return std::nullopt;
    }
    SkipLeadingWhitespace(sql, inner);
    if (inner > closeBrace) {
        return std::nullopt;
    }
    OdbcBraceParsed parsed;
    parsed.Kind = OdbcBraceKind::OutputProcedureCall;
    parsed.RecurseTail = std::string_view(sql.data() + inner, closeBrace - inner);
    return parsed;
}

std::optional<OdbcBraceParsed> MakeRecurseTailBrace(OdbcBraceKind kind, std::string_view sql, size_t& parsePos, size_t closeBrace) {
    SkipLeadingWhitespace(sql, parsePos);
    if (parsePos > closeBrace) {
        return std::nullopt;
    }
    OdbcBraceParsed parsed;
    parsed.Kind = kind;
    parsed.RecurseTail = std::string_view(sql.data() + parsePos, closeBrace - parsePos);
    return parsed;
}

std::optional<OdbcBraceParsed> MakeQuotedBrace(OdbcBraceKind kind, std::string_view sql, size_t& parsePos, size_t closeBrace) {
    std::string quotedLit;
    if (!ParseSingleQuoted(sql, parsePos, &quotedLit) || parsePos > closeBrace) {
        return std::nullopt;
    }
    SkipLeadingWhitespace(sql, parsePos);
    if (parsePos != closeBrace) {
        return std::nullopt;
    }
    OdbcBraceParsed parsed;
    parsed.Kind = kind;
    parsed.QuotedValue = std::move(quotedLit);
    return parsed;
}

struct BraceKeywordSpec {
    std::string_view Keyword;
    OdbcBraceKind Kind;
    bool IsQuotedLiteral;
};

static constexpr BraceKeywordSpec kBraceKeywordSpecs[] = {
    {"fn", OdbcBraceKind::FnBody, false},
    {"oj", OdbcBraceKind::OjBody, false},
    {"d", OdbcBraceKind::DateLiteral, true},
    {"t", OdbcBraceKind::TimeLiteral, true},
    {"ts", OdbcBraceKind::TimestampLiteral, true},
    {"call", OdbcBraceKind::ProcedureCall, false},
    {"escape", OdbcBraceKind::LikeEscape, true},
};

std::optional<OdbcBraceParsed> TryParseOdbcBrace(std::string_view sql, size_t openBrace, size_t closeBrace) {
    size_t parsePos = openBrace + 1;
    SkipLeadingWhitespace(sql, parsePos);

    if (std::optional<OdbcBraceParsed> outputCall = TryParseOutputCallBrace(sql, parsePos, closeBrace)) {
        return outputCall;
    }
    if (parsePos + 1 < sql.size() && sql[parsePos] == '?' && sql[parsePos + 1] == '=') {
        return std::nullopt;
    }

    std::string_view token;
    if (!ReadIdent(sql, parsePos, &token)) {
        return std::nullopt;
    }

    for (const BraceKeywordSpec& spec : kBraceKeywordSpecs) {
        if (!EqualNoCase(token, spec.Keyword)) {
            continue;
        }
        if (spec.IsQuotedLiteral) {
            return MakeQuotedBrace(spec.Kind, sql, parsePos, closeBrace);
        }
        return MakeRecurseTailBrace(spec.Kind, sql, parsePos, closeBrace);
    }

    return std::nullopt;
}

void AppendRewrittenBrace(std::string& rewritten, const OdbcBraceParsed& parsed) {
    switch (parsed.Kind) {
        case OdbcBraceKind::OutputProcedureCall:
        case OdbcBraceKind::ProcedureCall:
            rewritten += "CALL ";
            rewritten.append(RewriteOdbcEscapesImpl(parsed.RecurseTail));
            return;
        case OdbcBraceKind::FnBody:
        case OdbcBraceKind::OjBody:
            rewritten.append(RewriteOdbcEscapesImpl(parsed.RecurseTail));
            return;
        case OdbcBraceKind::DateLiteral:
            rewritten += "CAST('";
            rewritten += parsed.QuotedValue;
            rewritten += "' AS Date)";
            return;
        case OdbcBraceKind::TimeLiteral:
            rewritten += "CAST('";
            rewritten += parsed.QuotedValue;
            rewritten += "' AS Time)";
            return;
        case OdbcBraceKind::TimestampLiteral: {
            const std::string normalizedTs = NormalizeOdbcTimestampLiteral(parsed.QuotedValue);
            rewritten += "CAST('";
            rewritten += normalizedTs;
            rewritten += "' AS Datetime)";
            return;
        }
        case OdbcBraceKind::LikeEscape:
            rewritten += " ESCAPE '";
            rewritten += parsed.QuotedValue;
            rewritten += '\'';
            return;
    }
}

std::string RewriteOdbcEscapesImpl(std::string_view sql) {
    std::string rewritten;
    rewritten.reserve(sql.size());

    for (size_t readPos = 0; readPos < sql.size();) {
        if (sql[readPos] != '{') {
            rewritten.push_back(sql[readPos++]);
            continue;
        }

        const size_t closeBrace = FindMatchingCloseBrace(sql, readPos);
        if (closeBrace == std::string_view::npos) {
            rewritten.push_back(sql[readPos++]);
            continue;
        }

        if (std::optional<OdbcBraceParsed> parsedBrace = TryParseOdbcBrace(sql, readPos, closeBrace)) {
            AppendRewrittenBrace(rewritten, *parsedBrace);
            readPos = closeBrace + 1;
            continue;
        }

        rewritten.push_back(sql[readPos++]);
    }

    return rewritten;
}

std::string RewriteOdbcConvertCalls(std::string_view sql);

class TOdbcConvertCallRewriter {
public:
    explicit TOdbcConvertCallRewriter(std::string_view sql)
        : Sql_(sql) {
        Rewritten_.reserve(sql.size());
    }

    std::string TakeResult() && {
        return std::move(Rewritten_);
    }

    void Run() {
        while (SegmentStart_ < Sql_.size()) {
            const std::optional<size_t> convertKeywordPos = FindNextConvertKeyword(SegmentStart_);
            if (!convertKeywordPos) {
                Rewritten_.append(Sql_.substr(SegmentStart_));
                break;
            }
            Rewritten_.append(Sql_.substr(SegmentStart_, *convertKeywordPos - SegmentStart_));
            if (!TryRewriteConvertAt(*convertKeywordPos)) {
                break;
            }
        }
    }

private:
    static constexpr size_t kConvertTokenLen = 7;

    std::optional<size_t> FindNextConvertKeyword(size_t from) const {
        for (size_t probePos = from; probePos + kConvertTokenLen <= Sql_.size(); ++probePos) {
            if (!EqualNoCase(Sql_.substr(probePos, kConvertTokenLen), "CONVERT")) {
                continue;
            }
            size_t afterKeyword = probePos + kConvertTokenLen;
            SkipLeadingWhitespace(Sql_, afterKeyword);
            if (afterKeyword < Sql_.size() && Sql_[afterKeyword] == '(') {
                return probePos;
            }
        }
        return std::nullopt;
    }

    bool TryRewriteConvertAt(size_t convertKeywordPos) {
        size_t parsePos = convertKeywordPos + kConvertTokenLen;
        SkipLeadingWhitespace(Sql_, parsePos);
        if (parsePos >= Sql_.size() || Sql_[parsePos] != '(') {
            Rewritten_.append(Sql_.substr(convertKeywordPos, kConvertTokenLen));
            SegmentStart_ = convertKeywordPos + kConvertTokenLen;
            return true;
        }
        ++parsePos;

        int parenDepth = 1;
        const size_t firstArgStart = parsePos;
        std::optional<size_t> typeCommaPos;
        for (; parsePos < Sql_.size(); ++parsePos) {
            if (Sql_[parsePos] == '(') {
                ++parenDepth;
            } else if (Sql_[parsePos] == ')') {
                --parenDepth;
            } else if (Sql_[parsePos] == ',' && parenDepth == 1) {
                typeCommaPos = parsePos;
                break;
            }
        }
        if (!typeCommaPos) {
            Rewritten_.append(Sql_.substr(convertKeywordPos));
            return false;
        }

        const std::string_view firstArg(Sql_.data() + firstArgStart, *typeCommaPos - firstArgStart);
        parsePos = *typeCommaPos + 1;
        SkipLeadingWhitespace(Sql_, parsePos);
        const size_t sqlTypeStart = parsePos;
        const auto sqlTypeEnd = std::find_if_not(
            Sql_.begin() + static_cast<std::ptrdiff_t>(parsePos),
            Sql_.end(),
            [](unsigned char byte) {
                return std::isalpha(byte) != 0 || byte == '_';
            });
        parsePos = static_cast<size_t>(sqlTypeEnd - Sql_.begin());
        const std::string_view sqlTypeToken(Sql_.data() + sqlTypeStart, parsePos - sqlTypeStart);
        SkipLeadingWhitespace(Sql_, parsePos);
        if (parsePos >= Sql_.size() || Sql_[parsePos] != ')') {
            Rewritten_.append(Sql_.substr(convertKeywordPos));
            return false;
        }

        const std::string yqlType = MapSqlTypeToken(sqlTypeToken);
        Rewritten_ += "CAST(";
        Rewritten_ += RewriteOdbcConvertCalls(RewriteOdbcEscapesImpl(firstArg));
        Rewritten_ += " AS ";
        Rewritten_ += yqlType;
        Rewritten_ += ')';
        SegmentStart_ = parsePos + 1;
        return true;
    }

    std::string_view Sql_;
    std::string Rewritten_;
    size_t SegmentStart_ = 0;
};

std::string RewriteOdbcConvertCalls(std::string_view sql) {
    TOdbcConvertCallRewriter rewriter(sql);
    rewriter.Run();
    return std::move(rewriter).TakeResult();
}

} // namespace



std::string RewriteOdbcEscapes(const std::string& sql) {
    std::string afterBraceRewrite = RewriteOdbcEscapesImpl(sql);
    return RewriteOdbcConvertCalls(afterBraceRewrite);
}

} // namespace NYdb::NOdbc
