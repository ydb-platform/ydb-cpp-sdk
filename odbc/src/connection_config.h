#pragma once

#include <ydb-cpp-sdk/client/driver/driver.h>

#include <map>
#include <string>
#include <string_view>

namespace NYdb::NOdbc {

enum class EAuthenticationMode {
    Anonymous,
    Token,
    Static,
    Metadata,
    ServiceAccount,
    OAuth2,
    Environment,
};

using TConnectionParameters = std::map<std::string, std::string>;

struct TResolvedConnectionSettings {
    std::string Endpoint;
    std::string Database;
    std::string DataSourceName;
    TDriverConfig DriverConfig;
};

TConnectionParameters ParseAndNormalizeConnectionString(std::string_view connectionString);
TConnectionParameters ReadDsnParameters(std::string_view dsn);
void OverlayConnectionParameters(TConnectionParameters& destination, const TConnectionParameters& source);

TResolvedConnectionSettings ResolveConnectionSettings(
    TConnectionParameters parameters,
    std::string dataSourceName = {});

} // namespace NYdb::NOdbc
