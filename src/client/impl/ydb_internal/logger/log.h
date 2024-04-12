#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/library/logger/log.h>
=======
#include <src/library/logger/log.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

namespace NYdb {

TLogFormatter GetPrefixLogFormatter(const std::string& prefix);
std::string GetDatabaseLogPrefix(const std::string& database);

} // namespace NYdb
