#include "test_utils.h"

TEST(DescriptorApi, ImplicitImpRowDesc) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1 AS col", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLHDESC ird = SQL_NULL_HDESC;
    CHECK_ODBC_OK(SQLGetStmtAttr(stmt, SQL_ATTR_IMP_ROW_DESC, &ird, sizeof(ird), nullptr),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_NE(ird, nullptr);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(DescriptorApi, ImpRowDescCount) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1 AS a, 2 AS b", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLHDESC ird = SQL_NULL_HDESC;
    CHECK_ODBC_OK(SQLGetStmtAttr(stmt, SQL_ATTR_IMP_ROW_DESC, &ird, sizeof(ird), nullptr),
                  stmt, SQL_HANDLE_STMT);
    SQLSMALLINT descCount = 0;
    CHECK_ODBC_OK(SQLGetDescField(ird, 0, SQL_DESC_COUNT, &descCount, 0, nullptr), ird, SQL_HANDLE_DESC);
    SQLSMALLINT numCols = 0;
    CHECK_ODBC_OK(SQLNumResultCols(stmt, &numCols), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(descCount, numCols);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(DescriptorApi, AppRowDescBindCol) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 42 AS v", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLINTEGER value = 0;
    SQLLEN indicator = 0;
    CHECK_ODBC_OK(SQLBindCol(stmt, 1, SQL_C_LONG, &value, 0, &indicator), stmt, SQL_HANDLE_STMT);
    SQLHDESC ard = SQL_NULL_HDESC;
    CHECK_ODBC_OK(SQLGetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, &ard, sizeof(ard), nullptr),
                  stmt, SQL_HANDLE_STMT);
    SQLSMALLINT type = 0;
    SQLSMALLINT subType = 0;
    SQLLEN length = 0;
    CHECK_ODBC_OK(SQLGetDescRec(ard, 1, nullptr, 0, nullptr, &type, &subType, &length, nullptr, nullptr, nullptr),
                  ard, SQL_HANDLE_DESC);
    EXPECT_EQ(type, SQL_C_LONG);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(DescriptorApi, ExplicitDescAllocCopy) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    SQLHDESC src = SQL_NULL_HDESC;
    SQLHDESC dst = SQL_NULL_HDESC;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, dbc, &src), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, dbc, &dst), SQL_SUCCESS);
    SQLSMALLINT type = SQL_C_LONG;
    SQLLEN length = sizeof(SQLINTEGER);
    CHECK_ODBC_OK(SQLSetDescRec(src, 1, type, SQL_INTEGER, length, 0, 0, nullptr, nullptr, nullptr),
                  src, SQL_HANDLE_DESC);
    CHECK_ODBC_OK(SQLCopyDesc(src, dst), src, SQL_HANDLE_DESC);
    SQLSMALLINT outType = 0;
    SQLSMALLINT outSubType = 0;
    SQLLEN outLen = 0;
    CHECK_ODBC_OK(SQLGetDescRec(dst, 1, nullptr, 0, nullptr, &outType, &outSubType, &outLen, nullptr, nullptr, nullptr),
                  dst, SQL_HANDLE_DESC);
    EXPECT_EQ(outType, SQL_C_LONG);
    EXPECT_EQ(outLen, length);
    SQLFreeHandle(SQL_HANDLE_DESC, src);
    SQLFreeHandle(SQL_HANDLE_DESC, dst);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}
