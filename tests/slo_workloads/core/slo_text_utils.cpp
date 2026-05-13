#include "slo_text_utils.h"

#include <util/folder/path.h>
#include <util/random/random.h>

#include <string_view>

std::string SloJoinPath(const std::string& prefix, const std::string& path) {
    if (prefix.empty()) {
        return path;
    }
    TPathSplitUnix prefixPathSplit(prefix);
    prefixPathSplit.AppendComponent(path);
    return prefixPathSplit.Reconstruct();
}

std::string SloGetDatabaseFromConnectionString(const std::string& connectionString) {
    constexpr std::string_view databaseFlag = "/?database=";
    size_t pathIndex = connectionString.find(databaseFlag);
    if (pathIndex != std::string::npos) {
        return connectionString.substr(pathIndex + databaseFlag.size());
    }
    return {};
}

std::string SloGenerateRandomString(std::uint32_t minLength, std::uint32_t maxLength) {
    std::uint32_t length = minLength + RandomNumber<std::uint32_t>() % (maxLength - minLength);
    static const char* symbols = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result.push_back(symbols[RandomNumber<std::uint8_t>(61)]);
    }
    return result;
}
