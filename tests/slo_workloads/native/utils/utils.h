#pragma once

#include <tests/slo_workloads/core/constants.h>
#include <tests/slo_workloads/core/duration_meter.h>
#include <tests/slo_workloads/core/records.h>
#include <tests/slo_workloads/core/rps.h>
#include <tests/slo_workloads/core/shard.h>
#include <tests/slo_workloads/core/slo_text_utils.h>

#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/table/table.h>
#include <ydb-cpp-sdk/client/value/value.h>

#include <library/cpp/getopt/last_getopt.h>

#include <util/random/random.h>

#include <functional>
#include <map>
#include <string>

struct TDatabaseOptions {
    NYdb::TDriver& Driver;
    const std::string& Prefix;
};

struct TCommonOptions {
    // Executor options:
    TDatabaseOptions DatabaseOptions;
    std::uint32_t SecondsToRun = 10;
    std::uint32_t Rps = 10;
    std::uint32_t MaxInputThreads = 50;
    std::uint32_t MaxCallbackThreads = 50;
    std::uint32_t MaxInfly = 500;
    std::uint32_t MaxRetries = 50;
    TDuration ReactionTime = DefaultReactionTime;
    bool StopOnError = false;

    // Generator options:
    std::uint32_t MinLength = 20;
    std::uint32_t MaxLength = 200;

    // Output options:
    bool DontPushMetrics = false;
    std::string MetricsPushUrl = "http://localhost:9090/api/v1/otlp/v1/metrics";
};

struct TCreateOptions {
    TCommonOptions CommonOptions;
    std::uint32_t Count = 10000;
    std::uint32_t PackSize = 100;
};

struct TRunOptions {
    TCommonOptions CommonOptions;
    bool DontRunA = false;
    bool DontRunB = false;
    std::uint32_t Read_rps = 1000;
    std::uint32_t Write_rps = 10;
    TDuration ReadTimeout = DefaultReactionTime;
    TDuration WriteTimeout = DefaultReactionTime;
};

using TCreateCommand = std::function<int(TDatabaseOptions&, int, char**)>;
using TRunCommand = std::function<int(TDatabaseOptions&, int, char**)>;
using TCleanupCommand = std::function<int(TDatabaseOptions&, int)>;

int DoMain(int argc, char** argv, TCreateCommand create, TRunCommand run, TCleanupCommand cleanup);

std::string GetCmdList();

enum class ECommandType {
    Unknown,
    Create,
    Run,
    Cleanup,
    All,  // No free-arg passed: execute Create -> Run -> Cleanup in one process
};

ECommandType ParseCommand(const char* cmd);

inline std::string JoinPath(const std::string& prefix, const std::string& path) {
    return SloJoinPath(prefix, path);
}

inline std::string GetDatabase(const std::string& connectionString) {
    return SloGetDatabaseFromConnectionString(connectionString);
}

inline std::string GenerateRandomString(std::uint32_t minLength, std::uint32_t maxLength) {
    return SloGenerateRandomString(minLength, maxLength);
}

inline std::uint32_t GetSpecialId(std::uint32_t id) {
    return SloGetSpecialId(id);
}

inline std::uint32_t GetShardSpecialId(std::uint64_t shardNo) {
    return SloGetShardSpecialId(shardNo);
}

inline std::uint32_t GetHash(std::uint32_t value) {
    return SloGetHash(value);
}

inline TSloGeneratorOptions MakeGeneratorOptions(const TCommonOptions& opts) {
    TSloGeneratorOptions o;
    o.MinLength = opts.MinLength;
    o.MaxLength = opts.MaxLength;
    return o;
}

std::map<std::string, std::string> MakeNativeSloOtelResourceAttributes();
std::string NativeSloMeterSchemaVersion();

inline void RetryBackoff(
    NYdb::NTable::TTableClient& client,
    std::uint32_t retries,
    const NYdb::NTable::TTableClient::TOperationSyncFunc& func
) {
    TDuration delay = TDuration::Seconds(5);
    while (retries) {
        NYdb::TStatus status = client.RetryOperationSync(func);
        if (status.IsSuccess()) {
            return;
        }
        --retries;
        if (!retries) {
            Cerr << "Create request failed after all retries." << Endl;
            Cerr << status << Endl;
            NYdb::NStatusHelpers::ThrowOnError(status);
        }
        Cerr << "Create request failed. Sleeping for " << delay << Endl;
        Sleep(delay);
        delay *= 2;
    }
}

NYdb::TParams PackValuesToParamsAsList(const std::vector<NYdb::TValue>& items, const std::string name = "$items");

std::string YdbStatusToString(NYdb::EStatus status);

struct TTableStats {
    std::uint64_t RowCount = 0;
    std::uint32_t MaxId = 0;
};

TTableStats GetTableStats(TDatabaseOptions& dbOptions, const std::string& tableName);

bool ParseOptionsCreate(int argc, char** argv, TCreateOptions& createOptions);
bool ParseOptionsRun(int argc, char** argv, TRunOptions& runOptions);
