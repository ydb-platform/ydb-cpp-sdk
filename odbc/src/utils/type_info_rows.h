#pragma once

#include "cursor.h"

#include <sql.h>

namespace NYdb::NOdbc {

TTable BuildTypeInfoRows(SQLSMALLINT dataType);

} // namespace NYdb::NOdbc
