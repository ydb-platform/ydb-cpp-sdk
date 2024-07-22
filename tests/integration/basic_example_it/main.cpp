#include "basic_example.h"
#include "../config_ydb.h"

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
    std::unique_ptr<Response> response = ::Run(driver, path);

    if (!response->success) {
        std::cerr << std::format("The driver could not be started (path = {})", path) << std::endl;
        FAIL();
        driver.Stop(true);
    }

    std::string correct_output = "> Describe table: series\nColumn, name: series_id, type: Uint64?\nColumn, name: title, type: Utf8?\nColumn, name: series_info, type: Utf8?\nColumn, name: release_date, type: Uint64?\n> SelectSimple:\nSeries, Id: 1, Title: IT Crowd, Release date: 2006-02-03\n> SelectWithParams:\nSeason, Title: Season 3, Series title: Silicon Valley\n> PreparedSelect:\nEpisode 7, Title: To Build a Better Beta, Air date: Sun Jun 05, 2016\n> PreparedSelect:\nEpisode 8, Title: Bachman's Earnings Over-Ride, Air date: Sun Jun 12, 2016\n> MultiStep:\nEpisode 1, Season: 5, Title: Grow Fast or Die Slow, Air date: Sun Mar 25, 2018\nEpisode 2, Season: 5, Title: Reorientation, Air date: Sun Apr 01, 2018\nEpisode 3, Season: 5, Title: Chief Operating Officer, Air date: Sun Apr 08, 2018\n> PreparedSelect:\nEpisode 1, Title: TBD, Air date: Thu Jan 01, 1970\n> ScanQuerySelect:\nSeason, SeriesId: 1, SeasonId: 1, Title: Season 1, Air date: 2006-02-03\nSeason, SeriesId: 1, SeasonId: 2, Title: Season 2, Air date: 2007-08-24\nSeason, SeriesId: 1, SeasonId: 3, Title: Season 3, Air date: 2008-11-21\nSeason, SeriesId: 1, SeasonId: 4, Title: Season 4, Air date: 2010-06-25\n";
    ASSERT_EQ(response->result, correct_output);

    driver.Stop(true);
}
