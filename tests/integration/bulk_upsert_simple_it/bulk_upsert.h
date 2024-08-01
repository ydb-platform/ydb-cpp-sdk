#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>

using namespace NYdb;
using namespace NYdb::NTable;

struct TRunArgs {
    TDriver driver;
    std::string path;
};

struct TLogMessage {
    uint64_t pk;
    std::string App;
    std::string Host;
    TInstant Timestamp;
    uint32_t HttpCode;
    std::string Message;
};

class TYdbErrorException : public yexception {
public:
    TYdbErrorException(const NYdb::TStatus& status)
        : Status(status) {}

    NYdb::TStatus Status;
};

struct TStatistic {
    uint64_t sumApp;
    uint64_t sumHost;
    uint64_t rowCount;
};

TRunArgs GetRunArgs();
TStatus CreateTable(TTableClient& client, const std::string& table);
TStatistic GetLogBatch(uint64_t logOffset, std::vector<TLogMessage>& logBatch, uint32_t lastNumber);
TStatus WriteLogBatch(TTableClient& tableClient, const std::string& table, const std::vector<TLogMessage>& logBatch,
                   const TRetryOperationSettings& retrySettings);
TStatistic Select(TTableClient& client, const std::string& path);
void DropTable(TTableClient& client, const std::string& path);
