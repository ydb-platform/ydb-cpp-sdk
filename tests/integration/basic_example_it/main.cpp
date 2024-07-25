#include "basic_example.h"

#include <src/library/getopt/last_getopt.h>

#include <src/util/stream/file.h>

#include <cstdlib>
#include <format>

#include <gtest/gtest.h>

using namespace NLastGetopt;
using namespace NYdb;

void StopHandler(int) {
    exit(1);
}

TEST(Integration, BasicExample) {
    TOpts opts = TOpts::Default();
    
    std::string database = std::getenv( "DATABASE" );
    std::string endpoint = std::getenv( "ENDPOINT" );;
    std::string path;
    std::string certPath;

    if (path.empty()) {
        path = database;
    }

    auto driverConfig = TDriverConfig()
        .SetEndpoint(endpoint)
        .SetDatabase(database)
        .SetAuthToken(std::getenv("YDB_TOKEN") ? std::getenv("YDB_TOKEN") : "");

    if (!certPath.empty()) {
        std::string cert = TFileInput(certPath).ReadAll();
        driverConfig.UseSecureConnection(cert);
    }

    TDriver driver(driverConfig);

    if (!::Run(driver, path)) {
        FAIL();
        driver.Stop(true);
    }

    driver.Stop(true);
}
