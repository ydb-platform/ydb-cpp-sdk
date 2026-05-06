#include "test_utils.h"

TEST(TransactionApi, AutocommitDefaultOn) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    
    SQLUINTEGER autocommit;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(autocommit, SQL_AUTOCOMMIT_ON);
    
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(TransactionApi, AutocommitOnOffToggle) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0),
                  dbc, SQL_HANDLE_DBC);
    SQLUINTEGER autocommit;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(autocommit, SQL_AUTOCOMMIT_OFF);
    
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0),
                  dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, &autocommit, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(autocommit, SQL_AUTOCOMMIT_ON);
    
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(TransactionApi, AutocommitOffRollback) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_rollback", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"CREATE TABLE test_rollback (id Int32, value Int32, PRIMARY KEY (id))", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0),
                  dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"UPSERT INTO test_rollback (id, value) VALUES (1, 100)", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_ROLLBACK), dbc, SQL_HANDLE_DBC);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT value FROM test_rollback WHERE id = 1", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(TransactionApi, AutocommitOffCommit) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_commit", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"CREATE TABLE test_commit (id Int32, value Int32, PRIMARY KEY (id))", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0),
                  dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"UPSERT INTO test_commit (id, value) VALUES (1, 200)", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_COMMIT), dbc, SQL_HANDLE_DBC);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT value FROM test_commit WHERE id = 1", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER value;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &value, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(value, 200);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(TransactionApi, MultipleStatementsInManualTransaction) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_multi_stmt", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"CREATE TABLE test_multi_stmt (id Int32, value Int32, PRIMARY KEY (id))", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0),
                  dbc, SQL_HANDLE_DBC);
    for (int i = 1; i <= 5; ++i) {
        char query[256];
        snprintf(query, sizeof(query), "UPSERT INTO test_multi_stmt (id, value) VALUES (%d, %d)", i, i * 10);
        CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS), stmt, SQL_HANDLE_STMT);
    }
    CHECK_ODBC_OK(SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_COMMIT), dbc, SQL_HANDLE_DBC);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT COUNT(*) FROM test_multi_stmt", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER count;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &count, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(count, 5);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(TransactionApi, SQLEndTranOnEnv) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_env_tran", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"CREATE TABLE test_env_tran (id Int32, PRIMARY KEY (id))", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0),
                  dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"UPSERT INTO test_env_tran (id) VALUES (1)", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLEndTran(SQL_HANDLE_ENV, env, SQL_COMMIT), env, SQL_HANDLE_ENV);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(TransactionApi, SQLEndTranInvalid) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLEndTran(SQL_HANDLE_DBC, dbc, 999), SQL_ERROR);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(TransactionApi, TxnIsolationDefault) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    SQLUINTEGER isolation;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, &isolation, sizeof(isolation), nullptr), SQL_SUCCESS);
    ASSERT_EQ(isolation, SQL_TXN_SERIALIZABLE);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(TransactionApi, TxnIsolationUnsupportedInReadWrite) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLSetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)SQL_TXN_READ_COMMITTED, 0), SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(dbc, SQL_HANDLE_DBC), "HYC00"));
    ASSERT_EQ(SQLSetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)9999, 0), SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(dbc, SQL_HANDLE_DBC), "HY024"));
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}
