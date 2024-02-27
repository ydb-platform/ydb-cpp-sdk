#pragma once

#include <client/impl/ydb_internal/internal_header.h>

#include <library/cpp/logger/log.h>

namespace NYdb {

TLogFormatter GetPrefixLogFormatter(const std::string& prefix);
std::string GetDatabaseLogPrefix(const std::string& database);

} // namespace NYdb
