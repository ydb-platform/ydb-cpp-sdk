#include "test_utils.h"

#include <tests/common/iam_mocks/iam_grpc_mock_server.h>
#include <tests/common/iam_mocks/iam_http_assertions.h>
#include <tests/common/iam_mocks/iam_http_mock_server.h>
#include <tests/common/iam_mocks/iam_jwt_assertions.h>
#include <tests/common/iam_mocks/iam_test_keys.h>
#include <tests/unit/client/oauth2_token_exchange/helpers/test_token_exchange_server.h>

#include <util/folder/tempdir.h>
#include <util/stream/file.h>

#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>

using namespace NYdb::NTest;

namespace {

constexpr std::string_view RootToken = "root@builtin";

class TScopedEnvironmentVariable {
public:
    TScopedEnvironmentVariable(std::string_view name, std::string_view value)
        : Name_(name)
    {
        if (const char* oldValue = std::getenv(Name_.c_str())) {
            OldValue_ = oldValue;
        }
        setenv(Name_.c_str(), std::string(value).c_str(), 1);
    }

    ~TScopedEnvironmentVariable() {
        if (OldValue_) {
            setenv(Name_.c_str(), OldValue_->c_str(), 1);
        } else {
            unsetenv(Name_.c_str());
        }
    }

private:
    std::string Name_;
    std::optional<std::string> OldValue_;
};

class OdbcAuthentication : public ::testing::Test {
protected:
    void SetUp() override {
        const char* endpoint = std::getenv("YDB_ENDPOINT");
        const char* database = std::getenv("YDB_DATABASE");
        if (!endpoint || !database) {
            GTEST_SKIP() << "Authentication integration tests require the IAM-enabled YDB fixture";
        }
        Endpoint_ = endpoint;
        Database_ = database;
        AllocEnv(&Env_);
    }

    void TearDown() override {
        Disconnect();
        if (Env_ != SQL_NULL_HENV) {
            SQLFreeHandle(SQL_HANDLE_ENV, Env_);
        }
    }

    void Connect(std::string_view authenticationAttributes) {
        Disconnect();
        ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, Env_, &Dbc_), SQL_SUCCESS);
        std::string connectionString = "Driver=" ODBC_DRIVER_PATH ";Endpoint=" + Endpoint_ +
            ";Database=" + Database_ + ";" + std::string(authenticationAttributes);
        const SQLRETURN rc = SQLDriverConnect(
            Dbc_, nullptr, reinterpret_cast<SQLCHAR*>(connectionString.data()), SQL_NTS,
            nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT);
        CHECK_ODBC_OK(rc, Dbc_, SQL_HANDLE_DBC);
    }

    void Disconnect() {
        if (Dbc_ != SQL_NULL_HDBC) {
            SQLDisconnect(Dbc_);
            SQLFreeHandle(SQL_HANDLE_DBC, Dbc_);
            Dbc_ = SQL_NULL_HDBC;
        }
    }

    void Execute(std::string_view query) {
        SQLHSTMT statement = SQL_NULL_HSTMT;
        ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, Dbc_, &statement), SQL_SUCCESS);
        std::string queryString(query);
        const SQLRETURN rc = SQLExecDirect(
            statement, reinterpret_cast<SQLCHAR*>(queryString.data()), SQL_NTS);
        CHECK_ODBC_OK(rc, statement, SQL_HANDLE_STMT);
        SQLFreeHandle(SQL_HANDLE_STMT, statement);
    }

    void SelectOne() {
        Execute("SELECT 1");
    }

    SQLHENV Env_ = SQL_NULL_HENV;
    SQLHDBC Dbc_ = SQL_NULL_HDBC;
    std::string Endpoint_;
    std::string Database_;
};

} // namespace

TEST_F(OdbcAuthentication, TokenAndAccessTokenAlias) {
    ASSERT_NO_FATAL_FAILURE(Connect("AuthMode=Token;Token=root@builtin;"));
    ASSERT_NO_FATAL_FAILURE(SelectOne());

    ASSERT_NO_FATAL_FAILURE(Connect("AccessToken=root@builtin;"));
    ASSERT_NO_FATAL_FAILURE(SelectOne());
}

TEST_F(OdbcAuthentication, Anonymous) {
    ASSERT_NO_FATAL_FAILURE(Connect("AuthMode=Anonymous;"));
    ASSERT_NO_FATAL_FAILURE(SelectOne());
}

TEST_F(OdbcAuthentication, StaticUserAndPasswordAliases) {
    const char* user = std::getenv("YDB_ODBC_STATIC_USER");
    const char* password = std::getenv("YDB_ODBC_STATIC_PASSWORD");
    if (!user || !password) {
        GTEST_SKIP() << "Static authentication requires credentials provisioned by the IAM fixture";
    }

    ASSERT_NO_FATAL_FAILURE(Connect(
        "AuthMode=Static;UID=" + std::string(user) + ";PWD=" + std::string(password) + ";"));
    ASSERT_NO_FATAL_FAILURE(SelectOne());
}

TEST_F(OdbcAuthentication, MetadataService) {
    TMetadataServer server;
    server.SetResponse(HTTP_OK, MakeTokenResponse(std::string(RootToken), 3600));

    ASSERT_NO_FATAL_FAILURE(Connect("AuthMode=Metadata;MetadataHost=127.0.0.1;MetadataPort=" +
        std::to_string(server.Port) + ";"));
    ASSERT_NO_FATAL_FAILURE(SelectOne());

    EXPECT_GE(server.GetRequestCount(), 1);
    AssertMetadataRequestShape(server);
}

TEST_F(OdbcAuthentication, ServiceAccountFileAndAlias) {
    TIamTokenServiceStub stub;
    stub.SetResponseToken(std::string(RootToken));
    TIamGrpcServer server(&stub);
    ASSERT_TRUE(server.Start());

    TTempDir tempDirectory;
    const TString keyPath = tempDirectory.Path() / "service-account.json";
    TFileOutput(keyPath).Write(MakeJwtKeyFileContent());

    ASSERT_NO_FATAL_FAILURE(Connect("AuthMode=ServiceAccount;SaFile=" + std::string(keyPath) +
        ";IamEndpoint=grpc://" + server.Endpoint() + ";"));
    ASSERT_NO_FATAL_FAILURE(SelectOne());

    EXPECT_GE(stub.GetRequestCount(), 1);
    ASSERT_TRUE(stub.HasLastRequest());
    AssertIamJwt(stub.GetLastRequest().jwt());
}

TEST_F(OdbcAuthentication, OAuth2TokenExchangeFile) {
    TTestTokenExchangeServer server;
    server.Check.ExpectedInputParams.emplace("grant_type", "urn:ietf:params:oauth:grant-type:token-exchange");
    server.Check.ExpectedInputParams.emplace("requested_token_type", "urn:ietf:params:oauth:token-type:access_token");
    server.Check.ExpectedInputParams.emplace("subject_token", "odbc-subject-token");
    server.Check.ExpectedInputParams.emplace("subject_token_type", "urn:ietf:params:oauth:token-type:access_token");
    server.Check.Response =
        R"({"access_token":"root@builtin","token_type":"bearer","expires_in":3600})";

    TTempDir tempDirectory;
    const TString configPath = tempDirectory.Path() / "oauth2.json";
    TFileOutput(configPath).Write(
        R"({"subject-credentials":{"type":"Fixed","token":"odbc-subject-token","token-type":"urn:ietf:params:oauth:token-type:access_token"}})");

    ASSERT_NO_FATAL_FAILURE(Connect("AuthMode=OAuth2;OAuth2KeyFile=" + std::string(configPath) +
        ";IamEndpoint=" + server.GetEndpoint() + ";"));
    ASSERT_NO_FATAL_FAILURE(SelectOne());
    server.CheckExpectations();
}

TEST_F(OdbcAuthentication, EnvironmentAccessToken) {
    TScopedEnvironmentVariable serviceAccount("YDB_SERVICE_ACCOUNT_KEY_FILE_CREDENTIALS", "");
    TScopedEnvironmentVariable anonymous("YDB_ANONYMOUS_CREDENTIALS", "0");
    TScopedEnvironmentVariable metadata("YDB_METADATA_CREDENTIALS", "0");
    TScopedEnvironmentVariable oauth2("YDB_OAUTH2_KEY_FILE", "");
    TScopedEnvironmentVariable token("YDB_ACCESS_TOKEN_CREDENTIALS", RootToken);
    ASSERT_NO_FATAL_FAILURE(Connect("AuthMode=Environment;"));
    ASSERT_NO_FATAL_FAILURE(SelectOne());
}
