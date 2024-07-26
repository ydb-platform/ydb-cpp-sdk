#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>

#include <gtest/gtest.h>

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

NYdb::TParams GetTablesDataParams();

void CreateTables(TTableClient client, const std::string& path);
void DescribeTable(TTableClient client, const std::string& path, const std::string& name);
void ThrowOnError(const TStatus& status);
void PrintStatus(const TStatus& status);
TStatus FillTableDataTransaction(TSession session, const std::string& path);
std::string SelectSimple(TTableClient client, const std::string& path);
void UpsertSimple(TTableClient client, const std::string& path);
std::string SelectWithParams(TTableClient client, const std::string& path);
std::string PreparedSelect(TTableClient client, const std::string& path, ui32 seriesId, ui32 seasonId, ui32 episodeId);
std::string MultiStep(TTableClient client, const std::string& path);
void ExplicitTcl(TTableClient client, const std::string& path);
std::string PreparedSelect(TTableClient client, const std::string& path, ui32 seriesId, ui32 seasonId, ui32 episodeId);
std::string ScanQuerySelect(TTableClient client, const std::string& path);
