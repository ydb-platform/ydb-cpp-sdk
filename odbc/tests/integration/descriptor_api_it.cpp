#include "test_utils.h"

#include <array>

TEST(DescriptorApi, ImplicitImpRowDesc) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)
        "SELECT CAST(1 AS Int8), CAST(1 AS Uint8), CAST(1 AS Int16), CAST(1 AS Uint16), "
        "CAST(1 AS Int32), CAST(1 AS Uint32), CAST(1 AS Int64), CAST(1 AS Uint64)", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLHDESC ird = SQL_NULL_HDESC;
    CHECK_ODBC_OK(SQLGetStmtAttr(stmt, SQL_ATTR_IMP_ROW_DESC, &ird, sizeof(ird), nullptr),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_NE(ird, nullptr);
    const std::array<SQLLEN, 8> displaySizes = {4, 3, 6, 5, 11, 10, 20, 20};
    for (SQLUSMALLINT column = 1; column <= displaySizes.size(); ++column) {
        SQLLEN displaySize = 0, columnDisplaySize = 0;
        CHECK_ODBC_OK(SQLGetDescField(ird, column, SQL_DESC_DISPLAY_SIZE,
                                     &displaySize, 0, nullptr), ird, SQL_HANDLE_DESC);
        CHECK_ODBC_OK(SQLColAttribute(stmt, column, SQL_DESC_DISPLAY_SIZE, nullptr, 0,
                                     nullptr, &columnDisplaySize), stmt, SQL_HANDLE_STMT);
        EXPECT_EQ(displaySize, displaySizes[column - 1]);
        EXPECT_EQ(displaySize, columnDisplaySize);
        SQLSMALLINT isUnsigned = SQL_FALSE;
        SQLLEN columnIsUnsigned = SQL_FALSE;
        CHECK_ODBC_OK(SQLGetDescField(ird, column, SQL_DESC_UNSIGNED,
                                     &isUnsigned, 0, nullptr), ird, SQL_HANDLE_DESC);
        CHECK_ODBC_OK(SQLColAttribute(stmt, column, SQL_DESC_UNSIGNED, nullptr, 0,
                                     nullptr, &columnIsUnsigned), stmt, SQL_HANDLE_STMT);
        EXPECT_EQ(isUnsigned, column % 2 == 0 ? SQL_TRUE : SQL_FALSE);
        EXPECT_EQ(isUnsigned, columnIsUnsigned);
    }
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

TEST(DescriptorApi, FreeBoundDescriptorsRestoresOriginals) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLHDESC originalArd = SQL_NULL_HDESC, originalApd = SQL_NULL_HDESC;
    CHECK_ODBC_OK(SQLGetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, &originalArd, sizeof(originalArd), nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLGetStmtAttr(stmt, SQL_ATTR_APP_PARAM_DESC, &originalApd, sizeof(originalApd), nullptr), stmt, SQL_HANDLE_STMT);
    SQLINTEGER value = 0, parameter = 37;
    CHECK_ODBC_OK(SQLBindCol(stmt, 1, SQL_C_LONG, &value, sizeof(value), nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                                  0, 0, &parameter, sizeof(parameter), nullptr), stmt, SQL_HANDLE_STMT);
    SQLHDESC descriptor = SQL_NULL_HDESC;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, dbc, &descriptor), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, descriptor, 0), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_APP_PARAM_DESC, descriptor, 0), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, descriptor), SQL_SUCCESS);
    SQLHDESC restored = SQL_NULL_HDESC;
    CHECK_ODBC_OK(SQLGetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, &restored, sizeof(restored), nullptr), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(restored, originalArd);
    CHECK_ODBC_OK(SQLGetStmtAttr(stmt, SQL_ATTR_APP_PARAM_DESC, &restored, sizeof(restored), nullptr), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(restored, originalApd);
    CHECK_ODBC_OK(SQLPrepare(stmt, (SQLCHAR*)"SELECT ? AS value", SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecute(stmt), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    EXPECT_EQ(value, parameter);
    SQLSMALLINT count = 0;
    CHECK_ODBC_OK(SQLNumParams(stmt, &count), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(count, 1);
    CHECK_ODBC_OK(SQLNumResultCols(stmt, &count), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_STMT, stmt), SQL_SUCCESS);
    EXPECT_EQ(SQLDisconnect(dbc), SQL_SUCCESS);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_DBC, dbc), SQL_SUCCESS);
    EXPECT_EQ(SQLFreeHandle(SQL_HANDLE_ENV, env), SQL_SUCCESS);
}
