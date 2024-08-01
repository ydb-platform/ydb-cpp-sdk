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
    struct TPrimaryKeyLogMessage {
        std::string App;
        std::string Host;
        TInstant Timestamp;
        bool operator<(const TPrimaryKeyLogMessage& o) const;
    };

    TPrimaryKeyLogMessage pk;
    uint32_t HttpCode;
    std::string Message;
    bool operator<(const TLogMessage& o) const {return pk < o.pk;};
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
TStatus CreateLogTable(TTableClient& client, const std::string& table);
TStatistic GetLogBatch(uint64_t logOffset, std::vector<TLogMessage>& logBatch, std::set<TLogMessage::TPrimaryKeyLogMessage>& setMessage);
TStatus WriteLogBatch(TTableClient& tableClient, const std::string& table, const std::vector<TLogMessage>& logBatch,
                   const TRetryOperationSettings& retrySettings);
TStatistic Select(TTableClient& client, const std::string& path);
void DropTable(TTableClient& client, const std::string& path);
