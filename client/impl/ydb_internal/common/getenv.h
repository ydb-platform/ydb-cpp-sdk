#pragma once

#include <client/impl/ydb_internal/common/type_switcher.h>

namespace NYdb {

std::string GetStrFromEnv(const char* envVarName, const std::string& defaultValue = "");

} // namespace NYdb

