#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>

NYdb::TParams GetTablesDataParams();

using namespace NYdb;
using namespace NYdb::NTable;

struct RunArgs {
    TDriver driver;
    std::string path;
};

class TYdbErrorException : public yexception {
public:
    TYdbErrorException(const TStatus& status)
        : Status(status) {}

    TStatus Status;
};

void CreateTable(TTableClient client, const std::string& path);
void ThrowOnError(const TStatus& status);
TStatus FillTableDataTransaction(TSession& session, const std::string& path);
std::string SelectPaging(TTableClient client, const std::string& path, uint64_t pageLimit, std::string& lastCity, uint32_t& lastNumber);
