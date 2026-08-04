#include "util.h"

#include <cctype>
#include <cstdint>

namespace NYdb::NOdbc {

namespace {

void TrimInPlace(std::string& value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.erase(value.begin());
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
}

} // namespace

std::string GetString(SQLCHAR* str, SQLINTEGER length) {
    if (!str) {
        return {};
    }
    if (length == SQL_NTS) {
        return std::string(reinterpret_cast<const char*>(str));
    }
    if (length <= 0) {
        return {};
    }
    size_t size = static_cast<size_t>(length);
    if (str[size - 1] == 0) {
        --size;
    }
    return std::string(reinterpret_cast<const char*>(str), size);
}

std::string GetString(SQLWCHAR* str, SQLINTEGER length) {
    if (!str) {
        return {};
    }

    size_t size = 0;
    if (length == SQL_NTS) {
        while (str[size] != 0) {
            ++size;
        }
    } else if (length > 0) {
        size = static_cast<size_t>(length);
        if (str[size - 1] == 0) {
            --size;
        }
    } else {
        return {};
    }

    std::string result;
    result.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        uint32_t codePoint = str[i];
        if (codePoint >= 0xd800 && codePoint <= 0xdbff) {
            if (i + 1 < size && str[i + 1] >= 0xdc00 && str[i + 1] <= 0xdfff) {
                codePoint = 0x10000 + ((codePoint - 0xd800) << 10) + (str[++i] - 0xdc00);
            } else {
                codePoint = 0xfffd;
            }
        } else if (codePoint >= 0xdc00 && codePoint <= 0xdfff) {
            codePoint = 0xfffd;
        }

        if (codePoint <= 0x7f) {
            result.push_back(static_cast<char>(codePoint));
        } else if (codePoint <= 0x7ff) {
            result.push_back(static_cast<char>(0xc0 | (codePoint >> 6)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3f)));
        } else if (codePoint <= 0xffff) {
            result.push_back(static_cast<char>(0xe0 | (codePoint >> 12)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3f)));
        } else {
            result.push_back(static_cast<char>(0xf0 | (codePoint >> 18)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3f)));
        }
    }
    return result;
}

bool StartsWithPrefix(const char* s, size_t sLen, const char* prefix, size_t prefixLen) {
    if (sLen < prefixLen) {
        return false;
    }
    for (size_t i = 0; i < prefixLen; ++i) {
        if (std::tolower(static_cast<unsigned char>(s[i])) !=
            std::tolower(static_cast<unsigned char>(prefix[i]))) {
            return false;
        }
    }
    return true;
}

TConnectionStringEntries ParseConnectionStringEntries(std::string_view connectionString) {
    TConnectionStringEntries entries;
    size_t pos = 0;
    while (pos < connectionString.size()) {
        const size_t eq = connectionString.find('=', pos);
        if (eq == std::string::npos) {
            break;
        }
        std::string key(connectionString.substr(pos, eq - pos));
        TrimInPlace(key);
        if (key.empty()) {
            break;
        }

        size_t valueStart = eq + 1;
        size_t valueEnd = connectionString.size();
        if (valueStart < connectionString.size() && connectionString[valueStart] == '{') {
            ++valueStart;
            size_t braceDepth = 1;
            size_t i = valueStart;
            while (i < connectionString.size() && braceDepth > 0) {
                if (connectionString[i] == '{') {
                    ++braceDepth;
                } else if (connectionString[i] == '}') {
                    --braceDepth;
                    if (braceDepth == 0) {
                        valueEnd = i;
                        pos = i + 1;
                        if (pos < connectionString.size() && connectionString[pos] == ';') {
                            ++pos;
                        }
                        break;
                    }
                }
                ++i;
            }
            if (braceDepth != 0) {
                valueEnd = connectionString.size();
                pos = connectionString.size();
            }
            entries.emplace_back(
                std::move(key), std::string(connectionString.substr(valueStart, valueEnd - valueStart)));
            continue;
        }

        const size_t sc = connectionString.find(';', valueStart);
        if (sc != std::string::npos) {
            valueEnd = sc;
            pos = sc + 1;
        } else {
            pos = connectionString.size();
        }
        std::string val(connectionString.substr(valueStart, valueEnd - valueStart));
        TrimInPlace(val);
        entries.emplace_back(std::move(key), std::move(val));
    }
    return entries;
}

std::map<std::string, std::string> ParseConnectionString(std::string_view connectionString) {
    std::map<std::string, std::string> params;
    for (auto&& [key, value] : ParseConnectionStringEntries(connectionString)) {
        params[std::move(key)] = std::move(value);
    }
    return params;
}

} // namespace NYdb::NOdbc
