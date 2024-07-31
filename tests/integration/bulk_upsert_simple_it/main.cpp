#include <ydb-cpp-sdk/client/table/table.h>
#include <ydb-cpp-sdk/json_value/ydb_json_value.h>

#include <src/library/getopt/last_getopt.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <set>

using namespace NYdb;
using namespace NYdb::NTable;

constexpr size_t BATCH_SIZE = 1000;

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

uint32_t correctSumApp = 0;
uint32_t correctSumHost = 0;
std::set <TLogMessage> setMessage;

void GetLogBatch(uint64_t logOffset, std::vector<TLogMessage>& logBatch) {
    logBatch.clear();
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
            setMessage.insert(message);
        }
        
    }
}

bool WriteLogBatch(NYdb::NTable::TTableClient& tableClient, const std::string& table, const std::vector<TLogMessage>& logBatch,
                   const NYdb::NTable::TRetryOperationSettings& retrySettings)
{
    NYdb::TValueBuilder rows;
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
    auto bulkUpsertOperation = [table, rowsValue = rows.Build()](NYdb::NTable::TTableClient& tableClient) {
        NYdb::TValue r = rowsValue;
        auto status = tableClient.BulkUpsert(table, std::move(r));
        return status.GetValueSync();
    };

    auto status = tableClient.RetryOperationSync(bulkUpsertOperation, retrySettings);

    if (!status.IsSuccess()) {
        std::cerr << std::endl << "Write failed with status: " << (const NYdb::TStatus&)status << std::endl;
        return false;
    }
    return true;
}

bool CreateLogTable(NYdb::NTable::TTableClient& client, const std::string& table) {
    std::cerr << "Create table " << table << "\n";

    NYdb::NTable::TRetryOperationSettings settings;
    auto status = client.RetryOperationSync([&table](NYdb::NTable::TSession session) {
            auto tableDesc = NYdb::NTable::TTableBuilder()
                .AddNullableColumn("App", NYdb::EPrimitiveType::Utf8)
                .AddNullableColumn("Timestamp", NYdb::EPrimitiveType::Timestamp)
                .AddNullableColumn("Host", NYdb::EPrimitiveType::Utf8)
                .AddNullableColumn("HttpCode", NYdb::EPrimitiveType::Uint32)
                .AddNullableColumn("Message", NYdb::EPrimitiveType::Utf8)
                .SetPrimaryKeyColumns({"App", "Host", "Timestamp"})
                .Build();

            return session.CreateTable(table, std::move(tableDesc)).GetValueSync();
        }, settings);

    if (!status.IsSuccess()) {
        std::cerr << "Create table failed with status: " << status << std::endl;
        return false;
    }
    return true;
}

class TYdbErrorException : public yexception {
public:
    TYdbErrorException(const NYdb::TStatus& status)
        : Status(status) {}

    NYdb::TStatus Status;
};

static void ThrowOnError(const NYdb::TStatus& status) {
    if (!status.IsSuccess()) {
        throw TYdbErrorException(status) << status;
    }
}

static TStatus ScanQuerySelect(TTableClient client, const std::string& path, std::vector <TResultSet>& vectorResultSet) {    
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

struct TResultSelect {
    uint32_t sumApp;
    uint32_t sumHost;
    uint32_t rowCount;
};

TResultSelect ScanQuerySelect(TTableClient client, const std::string& path) {
    std::vector <TResultSet> vectorResultSet;
    ThrowOnError(client.RetryOperationSync([path, &vectorResultSet](TTableClient& client) {
        return ScanQuerySelect(client, path, vectorResultSet);
    }));

    uint32_t sumApp = 0;
    uint32_t sumHost = 0;
    uint32_t rowCount = 0;

    for (TResultSet& resultSet : vectorResultSet) {
        NYdb::TResultSetParser parser(resultSet);
        
        while (parser.TryNextRow()) {

            ++rowCount;
            sumApp += ToString(parser.ColumnParser("App").GetOptionalUtf8()).back() - '0';
            std::string strHost = ToString(parser.ColumnParser("Host").GetOptionalUtf8());
            char penCharStrHost = strHost[strHost.size() - 2];
            sumHost += strHost.back() - '0' + (penCharStrHost == '.' ? 0 : (penCharStrHost - '0') * 10 );
        }
        
    }
    
    std::cout << "\nCorrect sum app: " << correctSumApp << std::endl << "Correct sum host: " << correctSumHost << std::endl << "Set cnt: " << setMessage.size() << std::endl;
    std::cout << "Sum App: " << sumApp << std::endl << "Sum Host: " << sumHost << std::endl << "Cnt: " << rowCount << std::endl;

    return {sumApp, sumHost, rowCount};
}

static void DropTable(NYdb::NTable::TTableClient& client, const std::string& path) {
    ThrowOnError(client.RetryOperationSync([path](NYdb::NTable::TSession session) {
        return session.DropTable(path).ExtractValueSync();
    }));
}

int RunDropTables(NYdb::TDriver& driver, const std::string& path) {
    NYdb::NTable::TTableClient client(driver);
    DropTable(client, path);
    DropTable(client, path);
    return 0;
}

std::string JoinPath(const std::string& basePath, const std::string& path) {
    if (basePath.empty()) {
        return path;
    }

    std::filesystem::path prefixPathSplit(basePath);
    prefixPathSplit /= path;

    return prefixPathSplit;
}

struct TRunArgs {
    TDriver driver;
    std::string path;
};

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

TEST(Integration, BulkUpsert) {

    auto [driver, path] = GetRunArgs();

    TTableClient client(driver);
    uint32_t count = 1000;

    if (!CreateLogTable(client, path)) {
        FAIL();
    }

    NYdb::NTable::TRetryOperationSettings writeRetrySettings;
    writeRetrySettings
            .Idempotent(true)
            .MaxRetries(20);

    std::vector<TLogMessage> logBatch;
    for (uint32_t offset = 0; offset < count; ++offset) {
        GetLogBatch(offset, logBatch);
        if (!WriteLogBatch(client, path, logBatch, writeRetrySettings)) {
            FAIL();
        }
        std::cerr << ".";
    }

    auto [sumApp, sumHost, rowCount] = ScanQuerySelect(client, path);

    ASSERT_EQ(sumApp, correctSumApp);
    ASSERT_EQ(sumHost, correctSumHost);
    ASSERT_EQ(rowCount, setMessage.size());

    DropTable(client, path);
    driver.Stop(true);

}
