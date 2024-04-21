#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

#include <ydb-cpp-sdk/library/logger/log.h>

namespace NYdb {

TLogFormatter GetPrefixLogFormatter(const std::string& prefix);
std::string GetDatabaseLogPrefix(const std::string& database);

} // namespace NYdb
