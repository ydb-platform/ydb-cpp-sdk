#pragma once

#include <ydb-cpp-sdk/client/types/status/status.h>

namespace NYdb::NOdbc {

NYdb::TStatus StatusFrom(const NYdb::TStatus& ydbStatus);
bool IsSessionInvalidated(const NYdb::TStatus& status);

} // namespace NYdb::NOdbc
