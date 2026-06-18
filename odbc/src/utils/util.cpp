#include "util.h"

#include <cctype>

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

std::string GetString(SQLCHAR* str, SQLSMALLINT length) {
    if (!str) {
        return {};
    }
    if (length == SQL_NTS) {
        return std::string(reinterpret_cast<const char*>(str));
    }
    if (length <= 0) {
        return {};
    }
    return std::string(reinterpret_cast<const char*>(str), length);
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

std::map<std::string, std::string> ParseConnectionString(const std::string& connectionString) {
    std::map<std::string, std::string> params;
    size_t pos = 0;
    while (pos < connectionString.size()) {
        const size_t eq = connectionString.find('=', pos);
        if (eq == std::string::npos) {
            break;
        }
        std::string key = connectionString.substr(pos, eq - pos);
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
            params[key] = connectionString.substr(valueStart, valueEnd - valueStart);
            continue;
        }

        const size_t sc = connectionString.find(';', valueStart);
        if (sc != std::string::npos) {
            valueEnd = sc;
            pos = sc + 1;
        } else {
            pos = connectionString.size();
        }
        std::string val = connectionString.substr(valueStart, valueEnd - valueStart);
        TrimInPlace(val);
        params[key] = val;
    }
    return params;
}

} // namespace NYdb::NOdbc
