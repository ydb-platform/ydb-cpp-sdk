#pragma once

#include <client/impl/ydb_internal/common/type_switcher.h>

namespace NYdb {

TStringType GetStrFromEnv(const char* envVarName, const TStringType& defaultValue = "");

} // namespace NYdb

