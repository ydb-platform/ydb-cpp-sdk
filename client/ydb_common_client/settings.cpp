#include "settings.h"

#include <client/impl/ydb_internal/common/parser.h>

namespace NYdb {

TCommonClientSettings& TCommonClientSettings::AuthToken(const std::optional<std::string>& token) {
    return CredentialsProviderFactory(CreateOAuthCredentialsProviderFactory(token.GetRef()));
}

TCommonClientSettings GetClientSettingsFromConnectionString(const std::string& connectionString) {
    TCommonClientSettings settings;
    auto connectionInfo = ParseConnectionString(connectionString);
    settings.Database(connectionInfo.Database);
    settings.DiscoveryEndpoint(connectionInfo.Endpoint);
    settings.SslCredentials(TSslCredentials(connectionInfo.EnableSsl));
    return settings;
}

} // namespace NYdb
