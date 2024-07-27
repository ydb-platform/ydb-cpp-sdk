#include "basic_example.h"

#include <src/util/stream/file.h>
#include <ydb-cpp-sdk/json_value/ydb_json_value.h>

#include <cstdlib>
#include <format>

#include <gtest/gtest.h>

RunArgs GetRunArgs() {
    
    std::string database = std::getenv("YDB_DATABASE");
    std::string endpoint = std::getenv("YDB_ENDPOINT");

    auto driverConfig = TDriverConfig()
        .SetEndpoint(endpoint)
        .SetDatabase(database)
        .SetAuthToken(std::getenv("YDB_TOKEN") ? std::getenv("YDB_TOKEN") : "");

    TDriver driver(driverConfig);
    return {driver, database};
}

TEST(Integration, BasicExample) {
    
    auto [driver, path] = GetRunArgs();

    TTableClient client(driver);

    try {
        CreateTables(client, path);

        ThrowOnError(client.RetryOperationSync([path](TSession session) {
            return FillTableDataTransaction(session, path);
        }));

        std::string expectedResultSelectSimple = "{\"series_id\":1,\"title\":\"IT Crowd\",\"release_date\":\"2006-02-03\"}\n";
        std::string resultSelectSimple = SelectSimple(client, path);
        ASSERT_EQ(resultSelectSimple, expectedResultSelectSimple);
        
        UpsertSimple(client, path);

        std::string expectedResultSelectWithParams = "{\"season_title\":\"Season 3\",\"series_title\":\"Silicon Valley\"}\n";
        std::string resultSelectWithParms = SelectWithParams(client, path);
        ASSERT_EQ(resultSelectWithParms, expectedResultSelectWithParams);

        std::string expectedResultPreparedSelect1 = "{\"air_date\":16957,\"episode_id\":7,\"season_id\":3,\"series_id\":2,\"title\":\"To Build a Better Beta\"}\n";
        std::string resultPreparedSelect1 = PreparedSelect(client, path, 2, 3, 7);
        ASSERT_EQ(resultPreparedSelect1, expectedResultPreparedSelect1);

        std::string expectedResultPreparedSelect2 = "{\"air_date\":16964,\"episode_id\":8,\"season_id\":3,\"series_id\":2,\"title\":\"Bachman's Earnings Over-Ride\"}\n";
        std::string resultPreparedSelect2 = PreparedSelect(client, path, 2, 3, 8);
        ASSERT_EQ(resultPreparedSelect2, expectedResultPreparedSelect2);

        std::string expectedResultMultiStep = "{\"season_id\":5,\"episode_id\":1,\"title\":\"Grow Fast or Die Slow\",\"air_date\":17615}\n{\"season_id\":5,\"episode_id\":2,\"title\":\"Reorientation\",\"air_date\":17622}\n{\"season_id\":5,\"episode_id\":3,\"title\":\"Chief Operating Officer\",\"air_date\":17629}\n";
        std::string resultMultiStep = MultiStep(client, path);
        ASSERT_EQ(resultMultiStep, expectedResultMultiStep);

        ExplicitTcl(client, path);

        std::string expectedResultPreparedSelect3 = "{\"air_date\":0,\"episode_id\":1,\"season_id\":6,\"series_id\":2,\"title\":\"TBD\"}\n";
        std::string resultPreparedSelect3 = PreparedSelect(client, path, 2, 6, 1);
        ASSERT_EQ(resultPreparedSelect3, expectedResultPreparedSelect3);

        std::vector<std::string> expectedResultScanQuerySelect = {"{\"series_id\":1,\"season_id\":1,\"title\":\"Season 1\",\"first_aired\":\"2006-02-03\"}\n{\"series_id\":1,\"season_id\":2,\"title\":\"Season 2\",\"first_aired\":\"2007-08-24\"}\n{\"series_id\":1,\"season_id\":3,\"title\":\"Season 3\",\"first_aired\":\"2008-11-21\"}\n{\"series_id\":1,\"season_id\":4,\"title\":\"Season 4\",\"first_aired\":\"2010-06-25\"}\n"};
        std::vector<std::string> resultScanQuerySelect = ScanQuerySelect(client, path);
        ASSERT_EQ(resultScanQuerySelect, expectedResultScanQuerySelect);
    }
    catch (const TYdbErrorException& e) {
        std::cerr << "Execution failed due to fatal error:" << std::endl;
        driver.Stop(true);
        FAIL();
    }

    driver.Stop(true);
}
