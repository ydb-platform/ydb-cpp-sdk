#pragma once

#include <client/ydb_driver/driver.h>
#include <client/ydb_table/table.h>

NYdb::TParams GetTablesDataParams();

bool Run(const NYdb::TDriver& driver, const std::string& path);
