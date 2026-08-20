#include "test_utils.h"

TEST(ConnectionApi, AllocFreeEnvHandle) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_ENV, env), SQL_SUCCESS);
}

TEST(ConnectionApi, AllocFreeDbcHandle) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_DBC, dbc), SQL_SUCCESS);
    
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, AllocFreeHandleInvalid) {
    SQLHENV env;
    SQLRETURN rc = SQLAllocHandle(999, SQL_NULL_HANDLE, &env);
    ASSERT_TRUE(rc == SQL_ERROR || rc == SQL_INVALID_HANDLE);
}

TEST(ConnectionApi, SQLConnectWithDSN) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLConnect(dbc, (SQLCHAR*)"YDB", SQL_NTS, (SQLCHAR*)"", SQL_NTS, (SQLCHAR*)"", SQL_NTS),
                  dbc, SQL_HANDLE_DBC);
    
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, SQLDriverConnectComplete) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    SQLCHAR outStr[1024] = {};
    SQLSMALLINT outLen = -1;
    SQLRETURN rc = SQLDriverConnect(dbc, nullptr, (SQLCHAR*)kConnStr, SQL_NTS,
                                    outStr, sizeof(outStr), &outLen, SQL_DRIVER_COMPLETE);
    ASSERT_EQ(rc, SQL_SUCCESS) << GetOdbcError(dbc, SQL_HANDLE_DBC);
    EXPECT_STREQ(reinterpret_cast<char*>(outStr), kConnStr);
    EXPECT_EQ(outLen, static_cast<SQLSMALLINT>(std::strlen(kConnStr)));
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, SQLDriverConnectReportsOutputLengthWithoutBuffer) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    SQLSMALLINT outLen = -1;
    const SQLRETURN rc = SQLDriverConnect(
        dbc, nullptr, (SQLCHAR*)kConnStr, SQL_NTS,
        nullptr, 0, &outLen, SQL_DRIVER_NOPROMPT);
    ASSERT_EQ(rc, SQL_SUCCESS) << GetOdbcError(dbc, SQL_HANDLE_DBC);
    EXPECT_EQ(outLen, static_cast<SQLSMALLINT>(std::strlen(kConnStr)));
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, SQLDriverConnectNoPrompt) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    
    SQLRETURN rc = SQLDriverConnect(dbc, nullptr, (SQLCHAR*)kConnStr, SQL_NTS,
                                    nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT);
    CHECK_ODBC_OK(rc, dbc, SQL_HANDLE_DBC);
    
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, SQLDriverConnectInvalidConnString) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    
    SQLRETURN rc = SQLDriverConnect(dbc, nullptr, (SQLCHAR*)"InvalidParam=test", SQL_NTS,
                                    nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT);
    ASSERT_EQ(rc, SQL_ERROR);
    
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, SQLDriverConnectIgnoresUnrecognizedAttributes) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);

    SQLCHAR connectionString[] =
        "Driver=" ODBC_DRIVER_PATH
        ";Endpoint=localhost:2136;Database=/local;APP=PowerBI;WSID=desktop;Timeout=30;";
    const SQLRETURN rc = SQLDriverConnect(
        dbc, nullptr, connectionString, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT);
    ASSERT_EQ(rc, SQL_SUCCESS_WITH_INFO) << GetOdbcError(dbc, SQL_HANDLE_DBC);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(dbc, SQL_HANDLE_DBC), "01S00"));

    SQLHSTMT stmt;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT 1")), SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, SQLDriverConnectValidatesAuthenticationSettings) {
    SQLHENV env;
    AllocEnv(&env);

    const struct {
        const char* ConnectionString;
        const char* SqlState;
    } cases[] = {
        {"Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;AuthMode=None;", "28000"},
        {"Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;Token=a;UID=b;PWD=c;", "28000"},
        {"Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;AuthMode=Static;UID=b;", "28000"},
        {"Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;AuthMode=Metadata;MetadataPort=70000;", "HY024"},
        {"Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;AuthMode=ServiceAccount;SaFile=/missing/sa.json;", "08001"},
        {"Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;AuthMode=OAuth2;OAuth2KeyFile=/missing/oauth2.json;", "08001"},
        {"Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;ClientCertificate=client.pem;", "08001"},
        {"Driver=" ODBC_DRIVER_PATH ";Endpoint=grpc://localhost:2136;Database=/local;CaFile=ca.pem;", "HY024"},
        {"Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;RootCertificate=/missing/ca.pem;", "08001"},
    };

    for (const auto& testCase : cases) {
        SQLHDBC dbc;
        ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
        const SQLRETURN rc = SQLDriverConnect(
            dbc, nullptr, reinterpret_cast<SQLCHAR*>(const_cast<char*>(testCase.ConnectionString)), SQL_NTS,
            nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT);
        ASSERT_EQ(rc, SQL_ERROR) << testCase.ConnectionString;
        EXPECT_TRUE(SqlStatePrefix(GetOdbcError(dbc, SQL_HANDLE_DBC), testCase.SqlState))
            << testCase.ConnectionString << ": " << GetOdbcError(dbc, SQL_HANDLE_DBC);
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }

    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, SQLDriverConnectSupportsAliasesAndDsnOverlay) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);

    SQLCHAR connectionString[] =
        "DSN=YDB;Endpoint=grpc://127.0.0.1:2136;AuthMode=Token;AccessToken=ignored-by-anonymous-server;";
    CHECK_ODBC_OK(SQLDriverConnect(
        dbc, nullptr, connectionString, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT),
        dbc, SQL_HANDLE_DBC);

    SQLHSTMT stmt;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, SQLConnectMissingDSN) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    
    SQLRETURN rc = SQLConnect(dbc, (SQLCHAR*)"NONEXISTENT_DSN", SQL_NTS, (SQLCHAR*)"", SQL_NTS, (SQLCHAR*)"", SQL_NTS);
    ASSERT_EQ(rc, SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(dbc, SQL_HANDLE_DBC), "IM002"));
    
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, EnvAttrOdbcVersion) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), SQL_SUCCESS);
    ASSERT_NE(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, nullptr, 0), SQL_SUCCESS);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, EnvAttrOutputNts) {
    SQLHENV env;
    AllocEnv(&env);
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_OUTPUT_NTS, (void*)SQL_TRUE, 0), SQL_SUCCESS);
    ASSERT_NE(SQLSetEnvAttr(env, SQL_ATTR_OUTPUT_NTS, (void*)SQL_FALSE, 0), SQL_SUCCESS);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, ConnAttrAccessMode) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    SQLUINTEGER mode;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, &mode, sizeof(mode), nullptr), SQL_SUCCESS);
    ASSERT_EQ(mode, SQL_MODE_READ_WRITE);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER)SQL_MODE_READ_ONLY, 0),
                  dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, &mode, sizeof(mode), nullptr), SQL_SUCCESS);
    ASSERT_EQ(mode, SQL_MODE_READ_ONLY);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER)SQL_MODE_READ_WRITE, 0),
                  dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLSetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER)9999, 0), SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(dbc, SQL_HANDLE_DBC), "HY024"));
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, ConnAttrCurrentCatalog) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    char catalog[256];
    SQLINTEGER len;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, catalog, sizeof(catalog), &len), SQL_SUCCESS);
    ASSERT_STREQ(catalog, "/local");
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)"/local/test", SQL_NTS),
                  dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, catalog, sizeof(catalog), &len), SQL_SUCCESS);
    ASSERT_STREQ(catalog, "/local/test");
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ConnectionApi, ConnAttrCurrentCatalogAffectsQueries) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS `/local/cat_a/probe`", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"CREATE TABLE `/local/cat_a/probe` (id Int32, value Int32, PRIMARY KEY (id))", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"UPSERT INTO `/local/cat_a/probe` (id, value) VALUES (1, 100)", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS `/local/cat_b/probe`", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"CREATE TABLE `/local/cat_b/probe` (id Int32, value Int32, PRIMARY KEY (id))", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"UPSERT INTO `/local/cat_b/probe` (id, value) VALUES (1, 200)", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)"/local/cat_a", SQL_NTS),
                  dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT value FROM probe WHERE id = 1", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER value;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &value, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(value, 100);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)"/local/cat_b", SQL_NTS),
                  dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT value FROM probe WHERE id = 1", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &value, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(value, 200);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}
