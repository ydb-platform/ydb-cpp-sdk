#pragma once

#include <src/client/driver/driver.h>
#include <src/client/table/table.h>

NYdb::TParams GetTablesDataParams();

bool Run(const NYdb::TDriver& driver, const std::string& path);
