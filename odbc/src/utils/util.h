#pragma once

#include <ydb-cpp-sdk/client/types/status/status.h>

#include <sql.h>
#include <sqlext.h>

#include <map>
#include <string>

namespace NYdb::NOdbc {

std::string GetString(SQLCHAR* str, SQLSMALLINT length);

bool StartsWithPrefix(const char* s, size_t sLen, const char* prefix, size_t prefixLen);

std::map<std::string, std::string> ParseConnectionString(const std::string& connectionString);

} // namespace NYdb::NOdbc
