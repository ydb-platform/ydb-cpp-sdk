#pragma once

#include <ydb-cpp-sdk/client/types/status/status.h>

#include <sql.h>
#include <sqlext.h>

#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace NYdb::NOdbc {

std::string GetString(SQLCHAR* str, SQLINTEGER length);

std::string GetString(SQLWCHAR* str, SQLINTEGER length);

bool StartsWithPrefix(const char* s, size_t sLen, const char* prefix, size_t prefixLen);

using TConnectionStringEntries = std::vector<std::pair<std::string, std::string>>;

TConnectionStringEntries ParseConnectionStringEntries(std::string_view connectionString);

std::map<std::string, std::string> ParseConnectionString(std::string_view connectionString);

} // namespace NYdb::NOdbc
