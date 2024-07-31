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

TStatus CreateLogTable(TTableClient& client, const std::string& table) {
    std::cerr << "Create table " << table << "\n";

    TRetryOperationSettings settings;
    auto status = client.RetryOperationSync([&table](TSession session) {
            auto tableDesc = TTableBuilder()
                .AddNullableColumn("App", EPrimitiveType::Utf8)
                .AddNullableColumn("Timestamp", EPrimitiveType::Timestamp)
                .AddNullableColumn("Host", EPrimitiveType::Utf8)
                .AddNullableColumn("HttpCode", EPrimitiveType::Uint32)
                .AddNullableColumn("Message", EPrimitiveType::Utf8)
                .SetPrimaryKeyColumns({"App", "Host", "Timestamp"})
                .Build();

            return session.CreateTable(table, std::move(tableDesc)).GetValueSync();
        }, settings);

    return status;
}

TStatistic GetLogBatch(uint64_t logOffset, std::vector<TLogMessage>& logBatch, std::set<TLogMessage>& setMessage) {
    logBatch.clear();
    uint32_t correctSumApp = 0;
    uint32_t correctSumHost = 0;
    uint32_t correctRowCount = 0;

    for (size_t i = 0; i < BATCH_SIZE; ++i) {
        TLogMessage message;        
        message.pk.App = "App_" + ToString(logOffset % 10);
        message.pk.Host = "192.168.0." + ToString(logOffset % 11);
        message.pk.Timestamp = TInstant::Now() + TDuration::MilliSeconds(i % 1000);
        message.HttpCode = 200;
        message.Message = i % 2 ? "GET / HTTP/1.1" : "GET /images/logo.png HTTP/1.1";
        logBatch.emplace_back(message);

        if (!setMessage.contains(message)) {
            correctSumApp += logOffset % 10;
            correctSumHost += logOffset % 11;
            ++correctRowCount;
            setMessage.insert(message);
        }
        
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
                .AddMember("App").Utf8(message.pk.App)
                .AddMember("Host").Utf8(message.pk.Host)
                .AddMember("Timestamp").Timestamp(message.pk.Timestamp)
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

static TStatus ScanQuerySelect(TTableClient& client, const std::string& path, std::vector <TResultSet>& vectorResultSet) {    
    std::filesystem::path filesystemPath(path);
    auto query = std::format(R"(
        --!syntax_v1
        PRAGMA TablePathPrefix("{}");

        SELECT * 
        FROM {}
    )", filesystemPath.parent_path().c_str(), filesystemPath.filename().c_str());

    auto resultScanQuery = client.StreamExecuteScanQuery(query).GetValueSync();

    if (!resultScanQuery.IsSuccess()) {
        return resultScanQuery;
    }

    bool eos = false;

    while (!eos) {
        auto streamPart = resultScanQuery.ReadNext().ExtractValueSync();

        if (!streamPart.IsSuccess()) {
            eos = true;
            if (!streamPart.EOS()) {
                return streamPart;
            }
            continue;
        }

        if (streamPart.HasResultSet()) {
            auto rs = streamPart.ExtractResultSet();
            vectorResultSet.push_back(rs);
        }
    }
    return TStatus(EStatus::SUCCESS, NYql::TIssues());
}

TStatistic ScanQuerySelect(TTableClient& client, const std::string& path) {
    std::vector <TResultSet> vectorResultSet;
    ThrowOnError(client.RetryOperationSync([path, &vectorResultSet](TTableClient& client) {
        return ScanQuerySelect(client, path, vectorResultSet);
    }));

    uint32_t sumApp = 0;
    uint32_t sumHost = 0;
    uint32_t rowCount = 0;

    for (TResultSet& resultSet : vectorResultSet) {
        TResultSetParser parser(resultSet);
        
        while (parser.TryNextRow()) {

            ++rowCount;
            sumApp += ToString(parser.ColumnParser("App").GetOptionalUtf8()).back() - '0';
            std::string strHost = ToString(parser.ColumnParser("Host").GetOptionalUtf8());
            char penCharStrHost = strHost[strHost.size() - 2];
            sumHost += strHost.back() - '0' + (penCharStrHost == '.' ? 0 : (penCharStrHost - '0') * 10);
        }
        
    }

    return {sumApp, sumHost, rowCount};
}

void DropTable(TTableClient& client, const std::string& path) {
    ThrowOnError(client.RetryOperationSync([path](TSession session) {
        return session.DropTable(path).ExtractValueSync();
    }));
}
