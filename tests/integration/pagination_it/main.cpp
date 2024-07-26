#include "pagination.h"

#include <src/library/getopt/last_getopt.h>
#include <ydb-cpp-sdk/json_value/ydb_json_value.h>

#include <gtest/gtest.h>

using namespace NLastGetopt;
using namespace NYdb;

const uint32_t MaxPages = 10;

void StopHandler(int) {
    exit(1);
}

RunArgs getRunArgs() {
    TOpts opts = TOpts::Default();

    std::string database = std::getenv( "DATABASE" );
    std::string endpoint = std::getenv( "ENDPOINT" );
    std::string path;

    if (path.empty()) {
        path = database;
    }

    auto driverConfig = TDriverConfig()
        .SetEndpoint(endpoint)
        .SetDatabase(database)
        .SetAuthToken(std::getenv("YDB_TOKEN") ? std::getenv("YDB_TOKEN") : "");
    TDriver driver(driverConfig);
    
    return {driver, path};
}

TEST(Integration, Pagination) {
    
    std::vector<std::string> mapPagToJson = {
        "{\"address\":\"Кирова, 6\",\"city\":\"Кирс\",\"number\":3}\n{\"address\":\"Урицкого, 21\",\"city\":\"Котельнич\",\"number\":1}\n{\"address\":\"Октябрьская, 109\",\"city\":\"Котельнич\",\"number\":2}\n",
        "{\"address\":\"Советская, 153\",\"city\":\"Котельнич\",\"number\":3}\n{\"address\":\"Школьная, 2\",\"city\":\"Котельнич\",\"number\":5}\n{\"address\":\"Октябрьская, 91\",\"city\":\"Котельнич\",\"number\":15}\n",
        "{\"address\":\"Коммуны, 4\",\"city\":\"Нолинск\",\"number\":1}\n{\"address\":\"Федосеева, 2Б\",\"city\":\"Нолинск\",\"number\":2}\n{\"address\":\"Ст.Халтурина, 2\",\"city\":\"Орлов\",\"number\":1}\n",
        "{\"address\":\"Свободы, 4\",\"city\":\"Орлов\",\"number\":2}\n{\"address\":\"Гоголя, 25\",\"city\":\"Яранск\",\"number\":1}\n{\"address\":\"Кирова, 18\",\"city\":\"Яранск\",\"number\":2}\n",
        "{\"address\":\"Некрасова, 59\",\"city\":\"Яранск\",\"number\":3}\n",
        ""
    };

    auto [driver, path] = getRunArgs();

    TTableClient client(driver);

    try {
        CreateTable(client, path);

        ThrowOnError(client.RetryOperationSync([path](TSession session) {
            return FillTableDataTransaction(session, path);
        }));

        uint64_t limit = 3;
        std::string lastCity;
        uint32_t lastNumber = 0;
        uint32_t page = 0;
        std::string currentResultSet = " ";

        std::cout << "> Pagination, Limit=" << limit << std::endl;

        // show first MaxPages=10 pages:
        while (!currentResultSet.empty() && page <= MaxPages) {
            ++page;
            std::cout << "> Page " << page << ":" << std::endl;
            currentResultSet = SelectPaging(client, path, limit, lastCity, lastNumber);
            ASSERT_EQ(mapPagToJson[page - 1], currentResultSet);
        }
    }
    catch (const TYdbErrorException& e) {
        std::cerr << "Execution failed due to fatal error:" << std::endl;
        PrintStatus(e.Status);
        FAIL();
    }
}
