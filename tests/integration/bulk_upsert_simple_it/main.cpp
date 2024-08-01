#include "bulk_upsert.h"

#include <gtest/gtest.h>

#include <set>

TEST(Integration, BulkUpsert) {

    uint32_t correctSumApp = 0;
    uint32_t correctSumHost = 0;
    uint32_t correctRowCount = 0;
    std::set <TLogMessage::TPrimaryKeyLogMessage> setMessage;

    auto [driver, path] = GetRunArgs();

    TTableClient client(driver);
    uint32_t count = 1000;
    TStatus statusCreate = CreateLogTable(client, path);
    if (!statusCreate.IsSuccess()) {
        FAIL() << "Create table failed with status: " << statusCreate << std::endl;
    }

    TRetryOperationSettings writeRetrySettings;
    writeRetrySettings
            .Idempotent(true)
            .MaxRetries(20);

    std::vector<TLogMessage> logBatch;
    for (uint32_t offset = 0; offset < count; ++offset) {

        auto [batchSumApp, batchSumHost, batchRowCount] = GetLogBatch(offset, logBatch, setMessage);
        correctSumApp += batchSumApp;
        correctSumHost += batchSumHost;
        correctRowCount += batchRowCount;

        TStatus statusWrite = WriteLogBatch(client, path, logBatch, writeRetrySettings);
        if (!statusWrite.IsSuccess()) {
            FAIL() << "Write failed with status: " << statusWrite << std::endl;
        }
    }

    auto [sumApp, sumHost, rowCount] = Select(client, path);

    EXPECT_EQ(rowCount, correctRowCount);
    EXPECT_EQ(sumApp, correctSumApp);
    EXPECT_EQ(sumHost, correctSumHost);
    
    DropTable(client, path);
    driver.Stop(true);

}
