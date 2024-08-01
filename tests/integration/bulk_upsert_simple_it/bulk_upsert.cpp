#include "bulk_upsert.h"

#include <src/library/getopt/last_getopt.h>

#include <filesystem>

static constexpr size_t BATCH_SIZE = 1000;

static void ThrowOnError(const TStatus& status) {
    if (!status.IsSuccess()) {
        throw TYdbErrorException(status) << status;
    }
}

static std::string JoinPath(const std::string& basePath, const std::string& path) {
    if (basePath.empty()) {
        return path;
    }

    std::filesystem::path prefixPathSplit(basePath);
    prefixPathSplit /= path;

    return prefixPathSplit;
}

TRunArgs GetRunArgs() {
    
    std::string database = std::getenv("YDB_DATABASE");
    std::string endpoint = std::getenv("YDB_ENDPOINT");

    auto driverConfig = TDriverConfig()
        .SetEndpoint(endpoint)
        .SetDatabase(database)
        .SetAuthToken(std::getenv("YDB_TOKEN") ? std::getenv("YDB_TOKEN") : "");

    TDriver driver(driverConfig);
    return {driver, JoinPath(database, "bulk")};
}

TStatus CreateTable(TTableClient& client, const std::string& table) {
    std::cerr << "Create table " << table << "\n";

    TRetryOperationSettings settings;
    auto status = client.RetryOperationSync([&table](TSession session) {
            auto tableDesc = TTableBuilder()
                .AddNonNullableColumn("pk", EPrimitiveType::Uint64)
                .AddNullableColumn("App", EPrimitiveType::Utf8)
                .AddNullableColumn("Timestamp", EPrimitiveType::Timestamp)
                .AddNullableColumn("Host", EPrimitiveType::Utf8)
                .AddNullableColumn("HttpCode", EPrimitiveType::Uint32)
                .AddNullableColumn("Message", EPrimitiveType::Utf8)
                .SetPrimaryKeyColumns({"pk"})
                .Build();

            return session.CreateTable(table, std::move(tableDesc)).GetValueSync();
        }, settings);

    return status;
}

TStatistic GetLogBatch(uint64_t logOffset, std::vector<TLogMessage>& logBatch, uint32_t lastNumber) {
    logBatch.clear();
    uint32_t correctSumApp = 0;
    uint32_t correctSumHost = 0;
    uint32_t correctRowCount = 0;

    for (size_t i = 0; i < BATCH_SIZE; ++i) {
        TLogMessage message;
        message.pk = correctRowCount + lastNumber;        
        message.App = "App_" + std::to_string(logOffset % 10);
        message.Host = "192.168.0." + std::to_string(logOffset % 11);
        message.Timestamp = TInstant::Now() + TDuration::MilliSeconds(i % 1000);
        message.HttpCode = 200;
        message.Message = i % 2 ? "GET / HTTP/1.1" : "GET /images/logo.png HTTP/1.1";
        logBatch.emplace_back(message);

        correctSumApp += logOffset % 10;
        correctSumHost += logOffset % 11;
        ++correctRowCount;
        
    }
    return {correctSumApp, correctSumHost, correctRowCount};
}

TStatus WriteLogBatch(TTableClient& tableClient, const std::string& table, const std::vector<TLogMessage>& logBatch,
                   const TRetryOperationSettings& retrySettings) {
    TValueBuilder rows;
    rows.BeginList();
    for (const auto& message : logBatch) {
        rows.AddListItem()
                .BeginStruct()
                .AddMember("pk").Uint64(message.pk)
                .AddMember("App").Utf8(message.App)
                .AddMember("Host").Utf8(message.Host)
                .AddMember("Timestamp").Timestamp(message.Timestamp)
                .AddMember("HttpCode").Uint32(message.HttpCode)
                .AddMember("Message").Utf8(message.Message)
                .EndStruct();
    }
    rows.EndList();
    auto bulkUpsertOperation = [table, rowsValue = rows.Build()](TTableClient& tableClient) {
        TValue r = rowsValue;
        auto status = tableClient.BulkUpsert(table, std::move(r));
        return status.GetValueSync();
    };

    auto status = tableClient.RetryOperationSync(bulkUpsertOperation, retrySettings);
    return status;
}

static TStatus SelectTransaction(TSession session, const std::string& path,
    std::optional<TResultSet>& resultSet) {
    std::filesystem::path filesystemPath(path);
    auto query = std::format(R"(
        PRAGMA TablePathPrefix("{}");

        SELECT
        SUM(CAST(SUBSTRING(CAST(App as string), 4) as Int32)),
        SUM(CAST(SUBSTRING(CAST(Host as string), 10) as Int32)),
        COUNT(*)
        FROM {}
    )", filesystemPath.parent_path().string(), filesystemPath.filename().string());

    auto txControl =
        TTxControl::BeginTx(TTxSettings::SerializableRW())
        .CommitTx();

    auto result = session.ExecuteDataQuery(query, txControl).GetValueSync();

    if (result.IsSuccess()) {
        resultSet = result.GetResultSet(0);
    }

    return result;
}

TStatistic Select(TTableClient& client, const std::string& path) {
    std::optional<TResultSet> resultSet;
    ThrowOnError(client.RetryOperationSync([path, &resultSet](TSession session) {
        return SelectTransaction(session, path, resultSet);
    }));

    TResultSetParser parser(*resultSet);

    uint64_t sumApp = 0;
    uint64_t sumHost = 0;
    uint64_t rowCount = 0;

    if (parser.TryNextRow()) {

        sumApp = *parser.ColumnParser("column0").GetOptionalInt64();
        sumHost = *parser.ColumnParser("column1").GetOptionalInt64();
        rowCount = parser.ColumnParser("column2").GetUint64();
    }

    return {sumApp, sumHost, rowCount};
}

void DropTable(TTableClient& client, const std::string& path) {
    ThrowOnError(client.RetryOperationSync([path](TSession session) {
        return session.DropTable(path).ExtractValueSync();
    }));
}
