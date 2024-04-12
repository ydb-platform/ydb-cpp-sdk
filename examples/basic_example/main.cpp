#include "basic_example.h"

#include <src/library/getopt/last_getopt.h>

#include <src/util/stream/file.h>

#include <cstdlib>

using namespace NLastGetopt;
using namespace NYdb;

void StopHandler(int) {
    exit(1);
}

int main(int argc, char** argv) {
    TOpts opts = TOpts::Default();

    std::string endpoint;
    std::string database;
    std::string path;
    std::string certPath;

    opts.AddLongOption('e', "endpoint", "YDB endpoint").Required().RequiredArgument("HOST:PORT")
        .StoreResult(&endpoint);
    opts.AddLongOption('d', "database", "YDB database name").Required().RequiredArgument("PATH")
        .StoreResult(&database);
    opts.AddLongOption('p', "path", "Base path for tables").Optional().RequiredArgument("PATH")
        .StoreResult(&path);
    opts.AddLongOption('c', "cert", "Certificate path to use secure connection").Optional().RequiredArgument("PATH")
        .StoreResult(&certPath);

    signal(SIGINT, &StopHandler);
    signal(SIGTERM, &StopHandler);

    TOptsParseResult res(&opts, argc, argv);

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

    if (!Run(driver, path)) {
        driver.Stop(true);
        return 2;
    }

    driver.Stop(true);
    return 0;
}
