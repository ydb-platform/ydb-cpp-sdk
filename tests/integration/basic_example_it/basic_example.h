#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>

struct Response {
    bool success;
    std::string result;
};

NYdb::TParams GetTablesDataParams();

std::unique_ptr<Response> Run(const NYdb::TDriver& driver, const std::string& path);
