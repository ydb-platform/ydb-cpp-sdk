#pragma once

#include <src/client/ydb_driver/driver.h>
#include <src/client/ydb_table/table.h>

NYdb::TParams GetTablesDataParams();

bool Run(const NYdb::TDriver& driver, const std::string& path);
