#include "connection_config.h"

#include "utils/error_manager.h"
#include "utils/util.h"

#include <ydb-cpp-sdk/client/helpers/helpers.h>
#include <ydb-cpp-sdk/client/iam/iam.h>
#include <ydb-cpp-sdk/client/types/credentials/credentials.h>
#include <ydb-cpp-sdk/client/types/credentials/oauth2_token_exchange/from_file.h>

#include <odbcinst.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace NYdb::NOdbc {

namespace {

std::string ToLower(std::string_view value) {
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return result;
}

std::optional<std::string> CanonicalKey(std::string_view key) {
    const std::string lower = ToLower(key);
    if (lower == "driver") return "Driver";
    if (lower == "description") return "Description";
    if (lower == "dsn") return "DSN";
    if (lower == "server" || lower == "endpoint") return "Endpoint";
    if (lower == "database") return "Database";
    if (lower == "authmode") return "AuthMode";
    if (lower == "token" || lower == "accesstoken") return "Token";
    if (lower == "user" || lower == "uid") return "User";
    if (lower == "password" || lower == "pwd") return "Password";
    if (lower == "metadatahost") return "MetadataHost";
    if (lower == "metadataport") return "MetadataPort";
    if (lower == "serviceaccountkeyfile" || lower == "safile") return "ServiceAccountKeyFile";
    if (lower == "oauth2keyfile") return "OAuth2KeyFile";
    if (lower == "iamendpoint") return "IamEndpoint";
    if (lower == "rootcertificate" || lower == "cafile") return "RootCertificate";
    if (lower == "clientcertificate") return "ClientCertificate";
    if (lower == "clientprivatekey") return "ClientPrivateKey";
    return std::nullopt;
}

[[noreturn]] void ThrowInvalidAttribute(std::string_view attribute, std::string_view detail) {
    throw TOdbcException("HY024", 0, "Invalid connection string attribute " +
        std::string(attribute) + ": " + std::string(detail));
}

bool Has(const TConnectionParameters& parameters, std::string_view key) {
    return parameters.contains(std::string(key));
}

std::string_view Get(const TConnectionParameters& parameters, std::string_view key) {
    const auto it = parameters.find(std::string(key));
    return it == parameters.end() ? std::string_view{} : std::string_view(it->second);
}

std::string_view RequireNonEmpty(
    const TConnectionParameters& parameters,
    std::string_view key,
    std::string_view authMode)
{
    const auto value = Get(parameters, key);
    if (value.empty()) {
        throw TOdbcException("28000", 0, std::string(authMode) +
            " authentication requires " + std::string(key));
    }
    return value;
}

std::string ReadDsnValue(std::string_view dsn, std::string_view key) {
    const std::string dsnName(dsn);
    const std::string attribute(key);
    std::vector<char> buffer(256);
    while (buffer.size() <= 1024 * 1024) {
        const int length = SQLGetPrivateProfileString(
            dsnName.c_str(), attribute.c_str(), "", buffer.data(), static_cast<int>(buffer.size()), nullptr);
        if (length < 0) {
            return {};
        }
        if (static_cast<size_t>(length) + 1 < buffer.size()) {
            return std::string(buffer.data(), static_cast<size_t>(length));
        }
        buffer.resize(buffer.size() * 2);
    }
    throw TOdbcException("08001", 0, "DSN attribute is too large: " + attribute);
}

std::string ReadFile(std::string_view attribute, std::string_view path) {
    const std::string pathString(path);
    std::ifstream input(pathString, std::ios::binary);
    if (!input) {
        throw TOdbcException("08001", 0, "Unable to read " + std::string(attribute) + " file: " + pathString);
    }
    std::string content{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()};
    if (content.empty()) {
        throw TOdbcException("08001", 0, std::string(attribute) + " file is empty: " + pathString);
    }
    return content;
}

struct TEndpointSettings {
    std::string Endpoint;
    bool Secure = false;
    bool ExplicitlyInsecure = false;
};

TEndpointSettings ParseYdbEndpoint(std::string_view value) {
    constexpr std::string_view grpc = "grpc://";
    constexpr std::string_view grpcs = "grpcs://";
    if (value.starts_with(grpc)) {
        return {std::string(value.substr(grpc.size())), false, true};
    }
    if (value.starts_with(grpcs)) {
        return {std::string(value.substr(grpcs.size())), true, false};
    }
    if (value.find("://") != std::string::npos) {
        ThrowInvalidAttribute("Endpoint", "only grpc:// and grpcs:// protocols are supported");
    }
    return {std::string(value), false, false};
}

void ApplyIamEndpoint(TIamJwtFilename& params, std::string_view value) {
    if (value.empty()) {
        return;
    }
    constexpr std::string_view grpc = "grpc://";
    constexpr std::string_view grpcs = "grpcs://";
    if (value.starts_with(grpc)) {
        params.Endpoint = std::string(value.substr(grpc.size()));
        params.EnableSsl = false;
    } else if (value.starts_with(grpcs)) {
        params.Endpoint = std::string(value.substr(grpcs.size()));
        params.EnableSsl = true;
    } else if (value.find("://") != std::string::npos) {
        ThrowInvalidAttribute("IamEndpoint", "service-account IAM supports grpc:// and grpcs://");
    } else {
        params.Endpoint = std::string(value);
        params.EnableSsl = true;
    }
}

uint32_t ParseMetadataPort(std::string_view value) {
    if (value.empty()) {
        ThrowInvalidAttribute("MetadataPort", "value is empty");
    }
    uint32_t port = 0;
    const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), port);
    if (error != std::errc() || end != value.data() + value.size() || port == 0 || port > 65535) {
        ThrowInvalidAttribute("MetadataPort", "expected an integer from 1 to 65535");
    }
    return port;
}

EAuthenticationMode ParseAuthMode(std::string_view value) {
    const std::string mode = ToLower(value);
    if (mode == "anonymous") return EAuthenticationMode::Anonymous;
    if (mode == "token") return EAuthenticationMode::Token;
    if (mode == "static") return EAuthenticationMode::Static;
    if (mode == "metadata") return EAuthenticationMode::Metadata;
    if (mode == "serviceaccount") return EAuthenticationMode::ServiceAccount;
    if (mode == "oauth2") return EAuthenticationMode::OAuth2;
    if (mode == "environment") return EAuthenticationMode::Environment;
    throw TOdbcException("28000", 0, "Unknown authentication mode: " + std::string(value));
}

EAuthenticationMode ResolveAuthMode(const TConnectionParameters& parameters) {
    const bool token = Has(parameters, "Token");
    const bool staticCredentials = Has(parameters, "User") || Has(parameters, "Password");
    const bool metadata = Has(parameters, "MetadataHost") || Has(parameters, "MetadataPort");
    const bool serviceAccount = Has(parameters, "ServiceAccountKeyFile");
    const bool oauth2 = Has(parameters, "OAuth2KeyFile");
    const size_t familyCount = static_cast<size_t>(token) + static_cast<size_t>(staticCredentials) +
        static_cast<size_t>(metadata) + static_cast<size_t>(serviceAccount) + static_cast<size_t>(oauth2);

    EAuthenticationMode mode;
    if (Has(parameters, "AuthMode")) {
        mode = ParseAuthMode(Get(parameters, "AuthMode"));
    } else if (familyCount == 0) {
        if (Has(parameters, "IamEndpoint")) {
            throw TOdbcException("28000", 0, "IamEndpoint requires ServiceAccount or OAuth2 authentication");
        }
        mode = EAuthenticationMode::Anonymous;
    } else if (familyCount > 1) {
        throw TOdbcException("28000", 0, "Authentication mode is ambiguous");
    } else if (token) {
        mode = EAuthenticationMode::Token;
    } else if (staticCredentials) {
        mode = EAuthenticationMode::Static;
    } else if (metadata) {
        mode = EAuthenticationMode::Metadata;
    } else if (serviceAccount) {
        mode = EAuthenticationMode::ServiceAccount;
    } else {
        mode = EAuthenticationMode::OAuth2;
    }

    const bool modeMatchesFamily =
        (mode == EAuthenticationMode::Token && token && familyCount == 1) ||
        (mode == EAuthenticationMode::Static && staticCredentials && familyCount == 1) ||
        (mode == EAuthenticationMode::Metadata && (!familyCount || (metadata && familyCount == 1))) ||
        (mode == EAuthenticationMode::ServiceAccount && serviceAccount && familyCount == 1) ||
        (mode == EAuthenticationMode::OAuth2 && oauth2 && familyCount == 1) ||
        ((mode == EAuthenticationMode::Anonymous || mode == EAuthenticationMode::Environment) && familyCount == 0);
    if (!modeMatchesFamily) {
        throw TOdbcException("28000", 0, "Credential attributes conflict with the selected authentication mode");
    }
    if (Has(parameters, "IamEndpoint") && mode != EAuthenticationMode::ServiceAccount &&
        mode != EAuthenticationMode::OAuth2) {
        throw TOdbcException("28000", 0, "IamEndpoint is valid only for ServiceAccount or OAuth2 authentication");
    }
    return mode;
}

} // namespace

TConnectionParameters ParseAndNormalizeConnectionString(
    std::string_view connectionString,
    std::vector<std::string>& ignoredAttributes)
{
    TConnectionParameters parameters;
    for (const auto& [key, value] : ParseConnectionStringEntries(connectionString)) {
        const auto canonical = CanonicalKey(key);
        if (!canonical) {
            ignoredAttributes.push_back(key);
            continue;
        }
        parameters[*canonical] = value;
    }
    return parameters;
}

TConnectionParameters ReadDsnParameters(std::string_view dsn) {
    TConnectionParameters parameters;
    // Aliases are read first so the canonical spelling wins inside a DSN.
    static constexpr std::array<const char*, 23> keys = {
        "Server", "UID", "PWD", "AccessToken", "SaFile", "CaFile",
        "Driver", "Description", "Endpoint", "Database", "AuthMode", "Token",
        "User", "Password", "MetadataHost", "MetadataPort", "ServiceAccountKeyFile",
        "OAuth2KeyFile", "IamEndpoint", "RootCertificate", "ClientCertificate",
        "ClientPrivateKey", "DSN"};
    for (const char* key : keys) {
        std::string value = ReadDsnValue(dsn, key);
        if (!value.empty()) {
            parameters[*CanonicalKey(key)] = std::move(value);
        }
    }
    return parameters;
}

void OverlayConnectionParameters(TConnectionParameters& destination, const TConnectionParameters& source) {
    std::optional<EAuthenticationMode> selectedMode;
    if (Has(source, "AuthMode")) {
        selectedMode = ParseAuthMode(Get(source, "AuthMode"));
    } else {
        const bool token = Has(source, "Token");
        const bool staticCredentials = Has(source, "User") || Has(source, "Password");
        const bool metadata = Has(source, "MetadataHost") || Has(source, "MetadataPort");
        const bool serviceAccount = Has(source, "ServiceAccountKeyFile");
        const bool oauth2 = Has(source, "OAuth2KeyFile");
        const size_t familyCount = static_cast<size_t>(token) + static_cast<size_t>(staticCredentials) +
            static_cast<size_t>(metadata) + static_cast<size_t>(serviceAccount) + static_cast<size_t>(oauth2);
        if (familyCount == 1) {
            selectedMode = token ? EAuthenticationMode::Token
                : staticCredentials ? EAuthenticationMode::Static
                : metadata ? EAuthenticationMode::Metadata
                : serviceAccount ? EAuthenticationMode::ServiceAccount
                : EAuthenticationMode::OAuth2;
            destination.erase("AuthMode");
        }
    }

    if (selectedMode) {
        const auto belongsToSelectedMode = [selectedMode](std::string_view key) {
            switch (*selectedMode) {
                case EAuthenticationMode::Token:
                    return key == "Token";
                case EAuthenticationMode::Static:
                    return key == "User" || key == "Password";
                case EAuthenticationMode::Metadata:
                    return key == "MetadataHost" || key == "MetadataPort";
                case EAuthenticationMode::ServiceAccount:
                    return key == "ServiceAccountKeyFile" || key == "IamEndpoint";
                case EAuthenticationMode::OAuth2:
                    return key == "OAuth2KeyFile" || key == "IamEndpoint";
                case EAuthenticationMode::Anonymous:
                case EAuthenticationMode::Environment:
                    return false;
            }
            return false;
        };
        for (const std::string_view key : {
                "Token", "User", "Password", "MetadataHost", "MetadataPort",
                "ServiceAccountKeyFile", "OAuth2KeyFile", "IamEndpoint"}) {
            if (!belongsToSelectedMode(key)) {
                destination.erase(std::string(key));
            }
        }
    }

    for (const auto& [key, value] : source) {
        destination[key] = value;
    }
}

TResolvedConnectionSettings ResolveConnectionSettings(
    TConnectionParameters parameters,
    std::string dataSourceName)
{
    const std::string endpointValue(Get(parameters, "Endpoint"));
    const std::string database(Get(parameters, "Database"));
    if (endpointValue.empty() || database.empty()) {
        throw TOdbcException("08001", 0, "Missing Endpoint (or Server) or Database");
    }

    const TEndpointSettings endpoint = ParseYdbEndpoint(endpointValue);
    const bool hasRoot = Has(parameters, "RootCertificate");
    const bool hasClientCert = Has(parameters, "ClientCertificate");
    const bool hasClientKey = Has(parameters, "ClientPrivateKey");
    if (hasClientCert != hasClientKey) {
        throw TOdbcException("08001", 0,
            "ClientCertificate and ClientPrivateKey must be specified together");
    }
    const bool hasTlsFiles = hasRoot || hasClientCert;
    if (endpoint.ExplicitlyInsecure && hasTlsFiles) {
        ThrowInvalidAttribute("Endpoint", "grpc:// cannot be combined with TLS certificate attributes");
    }

    const EAuthenticationMode authMode = ResolveAuthMode(parameters);
    TDriverConfig driverConfig = authMode == EAuthenticationMode::Environment
        ? CreateFromEnvironment()
        : TDriverConfig();
    driverConfig.SetEndpoint(endpoint.Endpoint).SetDatabase(database);

    switch (authMode) {
        case EAuthenticationMode::Anonymous:
            driverConfig.SetCredentialsProviderFactory(CreateInsecureCredentialsProviderFactory());
            break;
        case EAuthenticationMode::Token:
            driverConfig.SetCredentialsProviderFactory(CreateOAuthCredentialsProviderFactory(
                std::string(RequireNonEmpty(parameters, "Token", "Token"))));
            break;
        case EAuthenticationMode::Static:
            driverConfig.SetCredentialsProviderFactory(CreateLoginCredentialsProviderFactory({
                .User = std::string(RequireNonEmpty(parameters, "User", "Static")),
                .Password = std::string(RequireNonEmpty(parameters, "Password", "Static")),
            }));
            break;
        case EAuthenticationMode::Metadata: {
            TIamHost params;
            if (Has(parameters, "MetadataHost")) {
                params.Host = std::string(RequireNonEmpty(parameters, "MetadataHost", "Metadata"));
            }
            if (Has(parameters, "MetadataPort")) {
                params.Port = ParseMetadataPort(Get(parameters, "MetadataPort"));
            }
            driverConfig.SetCredentialsProviderFactory(CreateIamCredentialsProviderFactory(params));
            break;
        }
        case EAuthenticationMode::ServiceAccount: {
            TIamJwtFilename params;
            params.JwtFilename = std::string(RequireNonEmpty(parameters, "ServiceAccountKeyFile", "ServiceAccount"));
            ApplyIamEndpoint(params, Get(parameters, "IamEndpoint"));
            try {
                driverConfig.SetCredentialsProviderFactory(CreateIamJwtFileCredentialsProviderFactory(params));
            } catch (const std::exception& ex) {
                throw TOdbcException("08001", 0,
                    "Unable to load ServiceAccountKeyFile " + params.JwtFilename + ": " + ex.what());
            }
            break;
        }
        case EAuthenticationMode::OAuth2: {
            const std::string path(RequireNonEmpty(parameters, "OAuth2KeyFile", "OAuth2"));
            try {
                driverConfig.SetCredentialsProviderFactory(
                    CreateOauth2TokenExchangeFileCredentialsProviderFactory(
                        path, std::string(Get(parameters, "IamEndpoint"))));
            } catch (const std::exception& ex) {
                throw TOdbcException("08001", 0,
                    "Unable to load OAuth2KeyFile " + path + ": " + ex.what());
            }
            break;
        }
        case EAuthenticationMode::Environment:
            break;
    }

    const bool secure = endpoint.Secure || hasTlsFiles;
    std::string rootPem;
    std::string clientCertPem;
    std::string clientKeyPem;
    if (hasRoot) {
        rootPem = ReadFile("RootCertificate", Get(parameters, "RootCertificate"));
    }
    if (hasClientCert) {
        clientCertPem = ReadFile("ClientCertificate", Get(parameters, "ClientCertificate"));
        clientKeyPem = ReadFile("ClientPrivateKey", Get(parameters, "ClientPrivateKey"));
    }
    if (secure) {
        driverConfig.UseSecureConnection(rootPem);
    }
    if (hasClientCert) {
        driverConfig.UseClientCertificate(clientCertPem, clientKeyPem);
    }

    if (dataSourceName.empty()) {
        dataSourceName = std::string(Get(parameters, "DSN"));
    }
    return {
        .Endpoint = endpoint.Endpoint,
        .Database = database,
        .DataSourceName = std::move(dataSourceName),
        .DriverConfig = std::move(driverConfig),
    };
}

} // namespace NYdb::NOdbc
