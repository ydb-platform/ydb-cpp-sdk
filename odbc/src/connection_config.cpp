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
#include <string_view>
#include <utility>
#include <vector>

namespace NYdb::NOdbc {
namespace {
enum class EAuthenticationMode : uint8_t {
    Anonymous = 0,
    Token = 1 << 0,
    Static = 1 << 1,
    Metadata = 1 << 2,
    ServiceAccount = 1 << 3,
    OAuth2 = 1 << 4,
    Environment = 1 << 5,
};
constexpr uint8_t AuthenticationFamilies = 0x1f;
constexpr uint8_t SelectsAuth = 1 << 7;
constexpr uint8_t IamAuthentication =
    uint8_t(EAuthenticationMode::ServiceAccount) | uint8_t(EAuthenticationMode::OAuth2);
struct TConnectionKeySpec {
    std::string_view Canonical;
    std::string_view Alias;
    uint8_t Authentication = 0;
};
constexpr std::array ConnectionKeys = {
    TConnectionKeySpec{"Driver"},
    TConnectionKeySpec{"Description"},
    TConnectionKeySpec{"DSN"},
    TConnectionKeySpec{"Endpoint", "Server"},
    TConnectionKeySpec{"Database"},
    TConnectionKeySpec{"AuthMode"},
    TConnectionKeySpec{"Token", "AccessToken", uint8_t(EAuthenticationMode::Token) | SelectsAuth},
    TConnectionKeySpec{"User", "UID", uint8_t(EAuthenticationMode::Static) | SelectsAuth},
    TConnectionKeySpec{"Password", "PWD", uint8_t(EAuthenticationMode::Static) | SelectsAuth},
    TConnectionKeySpec{"MetadataHost", {}, uint8_t(EAuthenticationMode::Metadata) | SelectsAuth},
    TConnectionKeySpec{"MetadataPort", {}, uint8_t(EAuthenticationMode::Metadata) | SelectsAuth},
    TConnectionKeySpec{"ServiceAccountKeyFile", "SaFile", uint8_t(EAuthenticationMode::ServiceAccount) | SelectsAuth},
    TConnectionKeySpec{"OAuth2KeyFile", {}, uint8_t(EAuthenticationMode::OAuth2) | SelectsAuth},
    TConnectionKeySpec{"IamEndpoint", {}, IamAuthentication},
    TConnectionKeySpec{"RootCertificate", "CaFile"},
    TConnectionKeySpec{"ClientCertificate"},
    TConnectionKeySpec{"ClientPrivateKey"},
};
constexpr std::array AuthModes = {
    std::pair{"Anonymous", EAuthenticationMode::Anonymous},
    std::pair{"Token", EAuthenticationMode::Token},
    std::pair{"Static", EAuthenticationMode::Static},
    std::pair{"Metadata", EAuthenticationMode::Metadata},
    std::pair{"ServiceAccount", EAuthenticationMode::ServiceAccount},
    std::pair{"OAuth2", EAuthenticationMode::OAuth2},
    std::pair{"Environment", EAuthenticationMode::Environment},
};

bool EqualsIgnoreCase(std::string_view lhs, std::string_view rhs) {
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin(), [](char lhs, char rhs) {
        return std::tolower(static_cast<unsigned char>(lhs)) == std::tolower(static_cast<unsigned char>(rhs));
    });
}
const TConnectionKeySpec* FindKey(std::string_view key) {
    const auto it = std::find_if(ConnectionKeys.begin(), ConnectionKeys.end(), [key](const auto& spec) {
        return EqualsIgnoreCase(key, spec.Canonical) || (!spec.Alias.empty() && EqualsIgnoreCase(key, spec.Alias));
    });
    return it == ConnectionKeys.end() ? nullptr : &*it;
}
[[noreturn]] void ThrowInvalidAttribute(std::string_view attribute, std::string_view detail) {
    throw TOdbcException("HY024", 0, "Invalid connection string attribute " +
        std::string(attribute) + ": " + std::string(detail));
}
bool Has(const TConnectionParameters& parameters, std::string_view key) {
    return parameters.contains(key);
}
std::string_view Get(const TConnectionParameters& parameters, std::string_view key) {
    const auto it = parameters.find(key);
    return it == parameters.end() ? std::string_view{} : std::string_view(it->second);
}

uint8_t CredentialMask(const TConnectionParameters& parameters) {
    uint8_t mask = 0;
    for (const auto& key : ConnectionKeys) {
        if ((key.Authentication & SelectsAuth) && Has(parameters, key.Canonical)) {
            mask |= key.Authentication & AuthenticationFamilies;
        }
    }
    return mask;
}

std::string_view RequireNonEmpty(
    const TConnectionParameters& parameters, std::string_view key, std::string_view authMode) {
    const auto value = Get(parameters, key);
    if (value.empty()) {
        throw TOdbcException("28000", 0, std::string(authMode) + " authentication requires " + std::string(key));
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
    std::string content(std::istreambuf_iterator<char>(input), {});
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

TEndpointSettings ParseEndpoint(
    std::string_view value,
    std::string_view attribute,
    bool secureByDefault,
    std::string_view protocolError = "only grpc:// and grpcs:// protocols are supported") {
    constexpr std::string_view grpc = "grpc://";
    constexpr std::string_view grpcs = "grpcs://";
    if (value.starts_with(grpc)) {
        return {std::string(value.substr(grpc.size())), false, true};
    }
    if (value.starts_with(grpcs)) {
        return {std::string(value.substr(grpcs.size())), true, false};
    }
    if (value.find("://") != std::string::npos) {
        ThrowInvalidAttribute(attribute, protocolError);
    }
    return {std::string(value), secureByDefault, false};
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
    for (const auto& [name, mode] : AuthModes) {
        if (EqualsIgnoreCase(value, name)) {
            return mode;
        }
    }
    throw TOdbcException("28000", 0, "Unknown authentication mode: " + std::string(value));
}

EAuthenticationMode ResolveAuthMode(const TConnectionParameters& parameters) {
    const uint8_t credentials = CredentialMask(parameters);
    EAuthenticationMode mode;
    if (Has(parameters, "AuthMode")) {
        mode = ParseAuthMode(Get(parameters, "AuthMode"));
    } else if (!credentials) {
        if (Has(parameters, "IamEndpoint")) {
            throw TOdbcException("28000", 0, "IamEndpoint requires ServiceAccount or OAuth2 authentication");
        }
        mode = EAuthenticationMode::Anonymous;
    } else if (credentials & (credentials - 1)) {
        throw TOdbcException("28000", 0, "Authentication mode is ambiguous");
    } else {
        mode = EAuthenticationMode(credentials);
    }

    const uint8_t family = uint8_t(mode) & AuthenticationFamilies;
    if (credentials != family && !(mode == EAuthenticationMode::Metadata && !credentials)) {
        throw TOdbcException("28000", 0, "Credential attributes conflict with the selected authentication mode");
    }
    if (Has(parameters, "IamEndpoint") && !(family & IamAuthentication)) {
        throw TOdbcException("28000", 0, "IamEndpoint is valid only for ServiceAccount or OAuth2 authentication");
    }
    return mode;
}
} // namespace

TConnectionParameters ParseAndNormalizeConnectionString(
    std::string_view connectionString, std::vector<std::string>& ignoredAttributes)
{
    TConnectionParameters parameters;
    for (const auto& [key, value] : ParseConnectionStringEntries(connectionString)) {
        const auto* spec = FindKey(key);
        if (!spec) {
            ignoredAttributes.push_back(key);
            continue;
        }
        parameters[std::string(spec->Canonical)] = value;
    }
    return parameters;
}

TConnectionParameters ReadDsnParameters(std::string_view dsn) {
    TConnectionParameters parameters;
    for (const auto& spec : ConnectionKeys) {
        for (const std::string_view key : {spec.Alias, spec.Canonical}) { // Canonical is read last and wins.
            if (!key.empty()) {
                if (std::string value = ReadDsnValue(dsn, key); !value.empty()) {
                    parameters[std::string(spec.Canonical)] = std::move(value);
                }
            }
        }
    }
    return parameters;
}

void OverlayConnectionParameters(TConnectionParameters& destination, const TConnectionParameters& source) {
    const bool explicitMode = Has(source, "AuthMode");
    uint8_t family = explicitMode
        ? uint8_t(ParseAuthMode(Get(source, "AuthMode"))) & AuthenticationFamilies
        : CredentialMask(source);
    const bool inferredMode = !explicitMode && family && !(family & (family - 1));
    if (inferredMode) {
        destination.erase("AuthMode");
    }
    if (explicitMode || inferredMode) {
        for (const auto& key : ConnectionKeys) {
            const uint8_t keyFamilies = key.Authentication & AuthenticationFamilies;
            if (keyFamilies && !(keyFamilies & family)) {
                destination.erase(std::string(key.Canonical));
            }
        }
    }
    for (const auto& [key, value] : source) {
        destination[key] = value;
    }
}

TResolvedConnectionSettings ResolveConnectionSettings(TConnectionParameters parameters, std::string dataSourceName) {
    const std::string endpointValue(Get(parameters, "Endpoint"));
    const std::string database(Get(parameters, "Database"));
    if (endpointValue.empty() || database.empty()) {
        throw TOdbcException("08001", 0, "Missing Endpoint (or Server) or Database");
    }

    const TEndpointSettings endpoint = ParseEndpoint(endpointValue, "Endpoint", false);
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
            if (const auto value = Get(parameters, "IamEndpoint"); !value.empty()) {
                auto iamEndpoint = ParseEndpoint(
                    value, "IamEndpoint", true,
                    "service-account IAM supports grpc:// and grpcs://");
                params.Endpoint = std::move(iamEndpoint.Endpoint);
                params.EnableSsl = iamEndpoint.Secure;
            }
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

    const std::string rootPem = hasRoot ? ReadFile("RootCertificate", Get(parameters, "RootCertificate")) : "";
    if (endpoint.Secure || hasTlsFiles) {
        driverConfig.UseSecureConnection(rootPem);
    }
    if (hasClientCert) {
        const std::string cert = ReadFile("ClientCertificate", Get(parameters, "ClientCertificate"));
        const std::string key = ReadFile("ClientPrivateKey", Get(parameters, "ClientPrivateKey"));
        driverConfig.UseClientCertificate(cert, key);
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
