#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>
=======
#include <src/client/ydb_driver/driver.h>
#include <src/client/ydb_table/table.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>
>>>>>>> origin/main

NYdb::TParams GetTablesDataParams();

bool Run(const NYdb::TDriver& driver, const std::string& path);
