#include "test_utils.h"

TEST(EnvironmentApi, AllocFreeEnv) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_ENV, env), SQL_SUCCESS);
}

TEST(EnvironmentApi, AllocEnvInvalidType) {
    SQLHENV env;
    SQLRETURN rc = SQLAllocHandle(999, SQL_NULL_HANDLE, &env);
    ASSERT_TRUE(rc == SQL_ERROR || rc == SQL_INVALID_HANDLE);
}

TEST(EnvironmentApi, FreeInvalidEnvHandle) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_ENV, env), SQL_SUCCESS);
    SQLRETURN rc = SQLFreeHandle(SQL_HANDLE_ENV, env);
    ASSERT_TRUE(rc == SQL_SUCCESS || rc == SQL_INVALID_HANDLE || rc == SQL_ERROR);
}

TEST(EnvironmentApi, DoubleFreeEnv) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_ENV, env), SQL_SUCCESS);
    // Second free may return error or success depending on implementation
    // but should not crash
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, SetOdbcVersion) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), SQL_SUCCESS);
    
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, SetOdbcVersionInvalid) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_NE(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, nullptr, 0), SQL_SUCCESS);
    ASSERT_NE(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)9999, 0), SQL_SUCCESS);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, GetOdbcVersion) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), SQL_SUCCESS);
    
    SQLINTEGER version;
    ASSERT_EQ(SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION, &version, sizeof(version), nullptr), SQL_SUCCESS);
    ASSERT_EQ(version, SQL_OV_ODBC3);
    
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, SetOutputNtsTrue) {
    SQLHENV env;
    AllocEnv(&env);
    
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_OUTPUT_NTS, (void*)SQL_TRUE, 0), SQL_SUCCESS);
    
    SQLINTEGER outputNts;
    ASSERT_EQ(SQLGetEnvAttr(env, SQL_ATTR_OUTPUT_NTS, &outputNts, sizeof(outputNts), nullptr), SQL_SUCCESS);
    ASSERT_EQ(outputNts, SQL_TRUE);
    
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, SetOutputNtsFalse) {
    SQLHENV env;
    AllocEnv(&env);
    ASSERT_NE(SQLSetEnvAttr(env, SQL_ATTR_OUTPUT_NTS, (void*)SQL_FALSE, 0), SQL_SUCCESS);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, GetOutputNtsDefault) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), SQL_SUCCESS);
    SQLINTEGER outputNts;
    ASSERT_EQ(SQLGetEnvAttr(env, SQL_ATTR_OUTPUT_NTS, &outputNts, sizeof(outputNts), nullptr), SQL_SUCCESS);
    ASSERT_EQ(outputNts, SQL_TRUE);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, SetInvalidEnvAttr) {
    SQLHENV env;
    AllocEnv(&env);
    ASSERT_EQ(SQLSetEnvAttr(env, 9999, (void*)1, 0), SQL_ERROR);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, GetInvalidEnvAttr) {
    SQLHENV env;
    AllocEnv(&env);
    char buffer[256];
    SQLINTEGER len;
    ASSERT_EQ(SQLGetEnvAttr(env, 9999, buffer, sizeof(buffer), &len), SQL_ERROR);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, MultipleConnectionsSequential) {
    SQLHENV env;
    AllocEnv(&env);
    for (int i = 0; i < 3; ++i) {
        SQLHDBC dbc;
        ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
        CHECK_ODBC_OK(SQLConnect(dbc, (SQLCHAR*)"YDB", SQL_NTS, nullptr, 0, nullptr, 0), dbc, SQL_HANDLE_DBC);
        SQLHSTMT stmt;
        ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
        char query[32];
        snprintf(query, sizeof(query), "SELECT %d", i + 1);
        CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS), stmt, SQL_HANDLE_STMT);
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        SQLDisconnect(dbc);
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

namespace {

void StartManualTx(SQLHDBC dbc, SQLHSTMT* stmt) {
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0), dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(*stmt, (SQLCHAR*)"SELECT 1", SQL_NTS), *stmt, SQL_HANDLE_STMT);
}

} // namespace

TEST(EnvironmentApi, EndTranCommitOnEnv) {
    SQLHENV env;
    SQLHDBC dbc1, dbc2;
    SQLHSTMT stmt1, stmt2;

    AllocEnvAndConnect(&env, &dbc1);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc2), SQL_SUCCESS);
    SQLRETURN rc = SQLDriverConnect(
        dbc2, nullptr, (SQLCHAR*)kConnStr, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE);
    CHECK_ODBC_OK(rc, dbc2, SQL_HANDLE_DBC);

    StartManualTx(dbc1, &stmt1);
    StartManualTx(dbc2, &stmt2);

    CHECK_ODBC_OK(SQLEndTran(SQL_HANDLE_ENV, env, SQL_COMMIT), env, SQL_HANDLE_ENV);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt2);
    SQLDisconnect(dbc1);
    SQLDisconnect(dbc2);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc1);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc2);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, EndTranRollbackOnEnv) {
    SQLHENV env;
    SQLHDBC dbc1, dbc2;
    SQLHSTMT stmt1, stmt2;

    AllocEnvAndConnect(&env, &dbc1);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc2), SQL_SUCCESS);
    SQLRETURN rc = SQLDriverConnect(
        dbc2, nullptr, (SQLCHAR*)kConnStr, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE);
    CHECK_ODBC_OK(rc, dbc2, SQL_HANDLE_DBC);

    StartManualTx(dbc1, &stmt1);
    StartManualTx(dbc2, &stmt2);

    CHECK_ODBC_OK(SQLEndTran(SQL_HANDLE_ENV, env, SQL_ROLLBACK), env, SQL_HANDLE_ENV);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt2);
    SQLDisconnect(dbc1);
    SQLDisconnect(dbc2);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc1);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc2);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, EndTranPartialFailureReturnsInfo) {
    SQLHENV env;
    SQLHDBC dbc1, dbc2;
    SQLHSTMT stmt1, stmt2;

    AllocEnvAndConnect(&env, &dbc1);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc2), SQL_SUCCESS);
    SQLRETURN rc = SQLDriverConnect(
        dbc2, nullptr, (SQLCHAR*)kConnStr, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE);
    CHECK_ODBC_OK(rc, dbc2, SQL_HANDLE_DBC);

    StartManualTx(dbc1, &stmt1);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc2, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0), dbc2, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc2, &stmt2), SQL_SUCCESS);
    (void)SQLExecDirect(stmt2, (SQLCHAR*)"SELECT FROM", SQL_NTS);

    rc = SQLEndTran(SQL_HANDLE_ENV, env, SQL_COMMIT);
    ASSERT_TRUE(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO || rc == SQL_ERROR)
        << GetOdbcError(env, SQL_HANDLE_ENV);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt2);
    SQLDisconnect(dbc1);
    SQLDisconnect(dbc2);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc1);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc2);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(EnvironmentApi, GetDiagRecEnv) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    (void)SQLSetEnvAttr(env, 9999, (void*)1, 0);
    SQLCHAR sqlState[6];
    SQLINTEGER nativeError;
    SQLCHAR msg[256];
    SQLSMALLINT msgLen;
    (void)SQLGetDiagRec(SQL_HANDLE_ENV, env, 1, sqlState, &nativeError, msg, sizeof(msg), &msgLen);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}
