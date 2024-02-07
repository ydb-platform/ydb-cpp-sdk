#pragma once

#include <client/ydb_types/credentials/credentials.h>

#include <memory>

namespace NKikimr {

struct TYdbCredentialsSettings {
    bool UseLocalMetadata = false;
    std::string OAuthToken;

    std::string SaKeyFile;
    std::string IamEndpoint;
};

using TYdbCredentialsProviderFactory = std::function<std::shared_ptr<NYdb::ICredentialsProviderFactory>(const TYdbCredentialsSettings& settings)>;

std::shared_ptr<NYdb::ICredentialsProviderFactory> CreateYdbCredentialsProviderFactory(const TYdbCredentialsSettings& settings);

}
