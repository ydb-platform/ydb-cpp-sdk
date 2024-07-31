#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>

using namespace NYdb;
using namespace NYdb::NTable;

struct TRunArgs {
    TDriver driver;
    std::string path;
};

class TLogMessage {
    struct TPrimaryKeyLogMessage {
        std::string App;
        std::string Host;
        TInstant Timestamp;
        auto operator<=>(const TPrimaryKeyLogMessage&) const = default;
    };

public:
    TPrimaryKeyLogMessage pk;
    uint32_t HttpCode;
    std::string Message;
    auto operator<=>(const TLogMessage& o) const {return pk <=> o.pk;};
};

class TYdbErrorException : public yexception {
public:
    TYdbErrorException(const NYdb::TStatus& status)
        : Status(status) {}

    NYdb::TStatus Status;
};

struct TStatistic {
    uint32_t sumApp;
    uint32_t sumHost;
    uint32_t rowCount;
};

TRunArgs GetRunArgs();
TStatus CreateLogTable(TTableClient& client, const std::string& table);
TStatistic GetLogBatch(uint64_t logOffset, std::vector<TLogMessage>& logBatch, std::set<TLogMessage>& setMessage);
TStatus WriteLogBatch(TTableClient& tableClient, const std::string& table, const std::vector<TLogMessage>& logBatch,
                   const TRetryOperationSettings& retrySettings);
TStatistic ScanQuerySelect(TTableClient& client, const std::string& path);
void DropTable(TTableClient& client, const std::string& path);