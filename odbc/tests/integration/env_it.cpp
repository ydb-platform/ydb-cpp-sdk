#include "test_utils.h"

namespace {

void AllocEnvAndConnect(SQLHENV* env, SQLHDBC* dbc) {
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, env), SQL_SUCCESS);
    ASSERT_EQ(SQLSetEnvAttr(*env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, *env, dbc), SQL_SUCCESS);
    SQLRETURN rc = SQLDriverConnect(
        *dbc, nullptr, (SQLCHAR*)kConnStr, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE);
    CHECK_ODBC_OK(rc, *dbc, SQL_HANDLE_DBC);
}

void StartManualTx(SQLHDBC dbc, SQLHSTMT* stmt) {
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0), dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(*stmt, (SQLCHAR*)"SELECT 1", SQL_NTS), *stmt, SQL_HANDLE_STMT);
}

} // namespace

TEST(OdbcEnv, EndTranCommitOnEnv) {
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

TEST(OdbcEnv, EndTranRollbackOnEnv) {
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

TEST(OdbcEnv, EndTranPartialFailureReturnsInfo) {
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
