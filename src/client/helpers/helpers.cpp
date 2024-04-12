#include <ydb-cpp-sdk/client/helpers/helpers.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/client/iam/iam.h>
#include <ydb-cpp-sdk/client/resources/ydb_ca.h>
=======
#include <src/client/iam/iam.h>
#include <src/client/resources/ydb_ca.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
#include <src/client/impl/ydb_internal/common/parser.h>
#include <src/client/impl/ydb_internal/common/getenv.h>
#include <src/util/stream/file.h>

namespace NYdb {

TDriverConfig CreateFromEnvironment(const std::string& connectionString) {
    TDriverConfig driverConfig;
    if (connectionString != "") {
        auto connectionInfo = ParseConnectionString(connectionString);
        driverConfig.SetEndpoint(connectionInfo.Endpoint);
        driverConfig.SetDatabase(connectionInfo.Database);
        if (connectionInfo.EnableSsl) {
            std::string rootCertsFile = GetStrFromEnv("YDB_SSL_ROOT_CERTIFICATES_FILE", "");
            std::string rootCerts = GetRootCertificate();
            if (rootCertsFile != "") {
                TFileInput in(rootCertsFile.c_str());
                rootCerts += in.ReadAll();
            }
            driverConfig.UseSecureConnection(rootCerts);
        }
    }

    std::string saKeyFile = GetStrFromEnv("YDB_SERVICE_ACCOUNT_KEY_FILE_CREDENTIALS", "");
    if (!saKeyFile.empty()) {
        driverConfig.SetCredentialsProviderFactory(
            CreateIamJwtFileCredentialsProviderFactory({.JwtFilename = saKeyFile}));
        return driverConfig;
    }
    bool anonymousUsed = GetStrFromEnv("YDB_ANONYMOUS_CREDENTIALS", "0") == "1";
    if (anonymousUsed) {
        return driverConfig;
    }

    bool useMetadataCredentials = GetStrFromEnv("YDB_METADATA_CREDENTIALS", "0") == "1";
    if (useMetadataCredentials){
        auto factory = CreateIamCredentialsProviderFactory();
        try {
            factory->CreateProvider();
        } catch (yexception& e) {
            ythrow yexception() << "Unable to get token from metadata service: " << e.what();
        }
        driverConfig.SetCredentialsProviderFactory(factory);
        return driverConfig;
    }

    std::string accessToken = GetStrFromEnv("YDB_ACCESS_TOKEN_CREDENTIALS", "");
    if (accessToken != ""){
        driverConfig.SetAuthToken(accessToken);
        return driverConfig;
    }

    ythrow yexception() << "Unable to create driver config from environment";
}

TDriverConfig CreateFromSaKeyFile(const std::string& saKeyFile, const std::string& connectionString) {
    TDriverConfig driverConfig(connectionString);
    driverConfig.SetCredentialsProviderFactory(
        CreateIamJwtFileCredentialsProviderFactory({.JwtFilename = saKeyFile}));
    return driverConfig;
}

} // namespace NYdb

