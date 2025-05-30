#pragma once

#include <sql.h>
#include <sqlext.h>

#include <string>

namespace NYdb::NOdbc {

std::string GetString(SQLCHAR* str, SQLSMALLINT length);

} // namespace NYdb::NOdbc
