#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>
=======
#include <src/client/ydb_driver/driver.h>
#include <src/client/ydb_table/table.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

NYdb::TParams GetTablesDataParams();

bool Run(const NYdb::TDriver& driver, const std::string& path);
