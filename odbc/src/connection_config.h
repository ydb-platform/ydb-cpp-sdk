#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace NYdb::NOdbc {
using TConnectionParameters = std::map<std::string, std::string, std::less<>>;
struct TResolvedConnectionSettings {
    std::string Endpoint;
    std::string Database;
    std::string DataSourceName;
    TDriverConfig DriverConfig;
};
TConnectionParameters ParseAndNormalizeConnectionString(
    std::string_view connectionString, std::vector<std::string>& ignoredAttributes);
TConnectionParameters ReadDsnParameters(std::string_view dsn);
void OverlayConnectionParameters(TConnectionParameters& destination, const TConnectionParameters& source);
TResolvedConnectionSettings ResolveConnectionSettings(
    TConnectionParameters parameters, std::string dataSourceName = {});
} // namespace NYdb::NOdbc
