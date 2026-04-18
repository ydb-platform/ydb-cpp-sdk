#include "test_utils.h"

#include <cstring>
#include <string>


TEST(OdbcAttrEnv, OdbcVersionAttr) {
    SQLHENV env;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), SQL_SUCCESS);
    ASSERT_NE(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, nullptr, 0), SQL_SUCCESS);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcAttrEnv, OutputNtsAttr) {
    SQLHENV env;
    AllocEnv(&env);
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_OUTPUT_NTS, (void*)SQL_TRUE, 0), SQL_SUCCESS);
    ASSERT_NE(SQLSetEnvAttr(env, SQL_ATTR_OUTPUT_NTS, (void*)SQL_FALSE, 0), SQL_SUCCESS);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcAttrConn, AutocommitAttr) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    SQLHSTMT stmt;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLCHAR dropQuery[] = "DROP TABLE IF EXISTS test_attr_autocommit";
    SQLCHAR createQuery[] =
        "CREATE TABLE test_attr_autocommit (id Int32, value Int32, PRIMARY KEY (id))";
    SQLCHAR upsertRollbackQuery[] = "UPSERT INTO test_attr_autocommit (id, value) VALUES (1, 100)";
    SQLCHAR upsertCommitQuery[] = "UPSERT INTO test_attr_autocommit (id, value) VALUES (1, 200)";
    SQLCHAR selectQuery[] = "SELECT value FROM test_attr_autocommit WHERE id = 1";

    CHECK_ODBC_OK(SQLExecDirect(stmt, dropQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, createQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);

    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0), dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLExecDirect(stmt, upsertRollbackQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_ROLLBACK), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, selectQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    CHECK_ODBC_OK(SQLExecDirect(stmt, upsertCommitQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_COMMIT), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, selectQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER valueInt = 0;
    SQLLEN valueInd = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &valueInt, 0, &valueInd), SQL_SUCCESS);
    ASSERT_EQ(valueInt, 200);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0), dbc, SQL_HANDLE_DBC);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcAttrConn, AccessModeAttr) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);

    constexpr SQLUINTEGER readWriteMode = SQL_MODE_READ_WRITE;
    constexpr SQLUINTEGER readOnlyMode = SQL_MODE_READ_ONLY;
    SQLUINTEGER currentMode = 0;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, &currentMode, sizeof(currentMode), nullptr), SQL_SUCCESS);
    ASSERT_EQ(readWriteMode, currentMode);

    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER)readOnlyMode, 0), dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, &currentMode, sizeof(currentMode), nullptr), SQL_SUCCESS);
    ASSERT_EQ(readOnlyMode, currentMode);

    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER)readWriteMode, 0), dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, &currentMode, sizeof(currentMode), nullptr), SQL_SUCCESS);
    ASSERT_EQ(readWriteMode, currentMode);

    ASSERT_EQ(SQLSetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER)9999, 0), SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(dbc, SQL_HANDLE_DBC), "HY024"));

    SQLHSTMT stmt;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLCHAR dropQuery[] = "DROP TABLE IF EXISTS test_attr_read_only";
    SQLCHAR createQuery[] = "CREATE TABLE test_attr_read_only (id Int32, PRIMARY KEY (id))";
    SQLCHAR selectOneQuery[] = "SELECT 1 AS value";
    SQLCHAR upsertQuery[] = "UPSERT INTO test_attr_read_only (id) VALUES (1)";
    CHECK_ODBC_OK(SQLExecDirect(stmt, dropQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, createQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER)readOnlyMode, 0), dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLExecDirect(stmt, selectOneQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLExecDirect(stmt, upsertQuery, SQL_NTS), SQL_ERROR);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcAttrConn, TxnIsolationAttr) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    SQLHSTMT stmt;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLCHAR selectOneQuery[] = "SELECT 1 AS value";

    SQLUINTEGER currentIsolation = 0;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, &currentIsolation, sizeof(currentIsolation), nullptr), SQL_SUCCESS);
    ASSERT_EQ(static_cast<SQLUINTEGER>(SQL_TXN_SERIALIZABLE), currentIsolation);

    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)SQL_TXN_REPEATABLE_READ, 0), dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLExecDirect(stmt, selectOneQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    ASSERT_EQ(SQLSetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)SQL_TXN_READ_COMMITTED, 0), SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(dbc, SQL_HANDLE_DBC), "HYC00"));
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, &currentIsolation, sizeof(currentIsolation), nullptr), SQL_SUCCESS);
    ASSERT_EQ(static_cast<SQLUINTEGER>(SQL_TXN_REPEATABLE_READ), currentIsolation);

    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER)SQL_MODE_READ_ONLY, 0), dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)SQL_TXN_READ_UNCOMMITTED, 0), dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLExecDirect(stmt, selectOneQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)SQL_TXN_READ_COMMITTED, 0), dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)SQL_TXN_REPEATABLE_READ, 0), dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)SQL_TXN_SERIALIZABLE, 0), dbc, SQL_HANDLE_DBC);

    ASSERT_EQ(SQLSetConnectAttr(dbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)9999, 0), SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(dbc, SQL_HANDLE_DBC), "HY024"));

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcAttrConn, CurrentCatalogAttr) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    SQLHSTMT stmt;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    constexpr const char* dbRoot = "/local";
    const std::string catA = std::string(dbRoot) + "/odbc_cat_a";
    const std::string catB = std::string(dbRoot) + "/odbc_cat_b";
    SQLCHAR dropAQuery[] = "DROP TABLE IF EXISTS `odbc_cat_a/probe`";
    SQLCHAR dropBQuery[] = "DROP TABLE IF EXISTS `odbc_cat_b/probe`";
    SQLCHAR createAQuery[] =
        "CREATE TABLE `odbc_cat_a/probe` (id Int32, value Int32, PRIMARY KEY (id))";
    SQLCHAR createBQuery[] =
        "CREATE TABLE `odbc_cat_b/probe` (id Int32, value Int32, PRIMARY KEY (id))";
    SQLCHAR upsertAQuery[] = "UPSERT INTO `odbc_cat_a/probe` (id, value) VALUES (1, 100)";
    SQLCHAR upsertBQuery[] = "UPSERT INTO `odbc_cat_b/probe` (id, value) VALUES (1, 200)";
    SQLCHAR selectAQuery[] = "SELECT value FROM `odbc_cat_a/probe` WHERE id = 1";
    SQLCHAR selectQuery[] = "SELECT value FROM probe WHERE id = 1";

    char catalog[256] = {0};
    SQLINTEGER textLen = 0;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, catalog, sizeof(catalog), &textLen), SQL_SUCCESS);
    ASSERT_STREQ(catalog, dbRoot);


    CHECK_ODBC_OK(SQLExecDirect(stmt, dropAQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, dropBQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, createAQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, createBQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, upsertAQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, upsertBQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, selectAQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    SQLINTEGER valueInt = 0;
    SQLLEN valueInd = 0;

    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)catA.c_str(), SQL_NTS), dbc,
        SQL_HANDLE_DBC);
    std::memset(catalog, 0, sizeof(catalog));
    textLen = 0;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, catalog, sizeof(catalog), &textLen), SQL_SUCCESS);
    ASSERT_STREQ(catalog, catA.c_str());
    CHECK_ODBC_OK(SQLExecDirect(stmt, selectQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &valueInt, 0, &valueInd), SQL_SUCCESS);
    ASSERT_EQ(valueInt, 100);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    valueInt = 0;
    valueInd = 0;
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)catB.c_str(), SQL_NTS), dbc,
        SQL_HANDLE_DBC);
    std::memset(catalog, 0, sizeof(catalog));
    textLen = 0;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, catalog, sizeof(catalog), &textLen), SQL_SUCCESS);
    ASSERT_STREQ(catalog, catB.c_str());
    CHECK_ODBC_OK(SQLExecDirect(stmt, selectQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &valueInt, 0, &valueInd), SQL_SUCCESS);
    ASSERT_EQ(valueInt, 200);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    const std::string catWithSlashes = catB + "///";
    CHECK_ODBC_OK(SQLSetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)catWithSlashes.c_str(), SQL_NTS), dbc,
        SQL_HANDLE_DBC);
    std::memset(catalog, 0, sizeof(catalog));
    textLen = 0;
    ASSERT_EQ(SQLGetConnectAttr(dbc, SQL_ATTR_CURRENT_CATALOG, catalog, sizeof(catalog), &textLen), SQL_SUCCESS);
    ASSERT_STREQ(catalog, catB.c_str());

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

