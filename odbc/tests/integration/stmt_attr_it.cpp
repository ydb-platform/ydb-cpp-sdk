#include "test_utils.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

#ifndef SQL_ATTR_METADATA_ID
#define SQL_ATTR_METADATA_ID 10029
#endif


TEST(OdbcStmtAttr, QueryTimeoutAttr) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLUINTEGER timeoutSec = 1;
    CHECK_ODBC_OK(
        SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)(uintptr_t)timeoutSec, 0),
        stmt,
        SQL_HANDLE_STMT);

    SQLCHAR longQuery[] =
        "SELECT COUNT(*) FROM AS_TABLE(ListMap(ListFromRange(1u, 100000000u), ($x)->(AsStruct($x AS v))))";
    ASSERT_EQ(SQLExecDirect(stmt, longQuery, SQL_NTS), SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "HYT00"));

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcStmtAttr, MaxRowsAttr) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLCHAR dropQuery[] = "DROP TABLE IF EXISTS test_attr_max_rows";
    SQLCHAR createQuery[] =
        "CREATE TABLE test_attr_max_rows (id Int32, value Int32, PRIMARY KEY (id))";
    SQLCHAR upsert1Query[] = "UPSERT INTO test_attr_max_rows (id, value) VALUES (1, 10)";
    SQLCHAR upsert2Query[] = "UPSERT INTO test_attr_max_rows (id, value) VALUES (2, 20)";
    SQLCHAR selectQuery[] = "SELECT value FROM test_attr_max_rows ORDER BY id";

    CHECK_ODBC_OK(SQLExecDirect(stmt, dropQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, createQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, upsert1Query, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, upsert2Query, SQL_NTS), stmt, SQL_HANDLE_STMT);

    const SQLULEN maxRows = 1;
    CHECK_ODBC_OK(
        SQLSetStmtAttr(stmt, SQL_ATTR_MAX_ROWS, (SQLPOINTER)(uintptr_t)maxRows, 0),
        stmt,
        SQL_HANDLE_STMT);

    SQLULEN maxRowsOut = 0;
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_MAX_ROWS, &maxRowsOut, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(maxRowsOut, maxRows);

    CHECK_ODBC_OK(SQLExecDirect(stmt, selectQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcStmtAttr, NoScanAttr) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLCHAR selectEscapeFnQuery[] = "SELECT {fn ABS(-12)} AS value";

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_OFF, 0), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, selectEscapeFnQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER valueInt = 0;
    SQLLEN valueInd = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &valueInt, 0, &valueInd), SQL_SUCCESS);
    ASSERT_EQ(valueInt, 12);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_ON, 0), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLExecDirect(stmt, selectEscapeFnQuery, SQL_NTS), SQL_ERROR);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcStmtAttr, OdbcEscapeSequences) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_OFF, 0), stmt, SQL_HANDLE_STMT);

    {
        SQLCHAR convertQuery[] = "SELECT {fn CONVERT(42, SQL_SMALLINT)} AS value";
        CHECK_ODBC_OK(SQLExecDirect(stmt, convertQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        SQLSMALLINT valueSmall = 0;
        SQLLEN valueInd = 0;
        ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_SSHORT, &valueSmall, 0, &valueInd), SQL_SUCCESS);
        ASSERT_EQ(valueSmall, 42);
        ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    }

    {
        SQLCHAR convertDoubleQuery[] = "SELECT {fn CONVERT(2.5, SQL_DOUBLE)} AS value";
        CHECK_ODBC_OK(SQLExecDirect(stmt, convertDoubleQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        double valueDouble = 0;
        SQLLEN valueInd = 0;
        ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_DOUBLE, &valueDouble, 0, &valueInd), SQL_SUCCESS);
        ASSERT_LT(std::fabs(valueDouble - 2.5), 1e-9);
        ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    }

    {
        SQLCHAR nestedFnQuery[] = "SELECT {fn {fn ABS(-10)}} AS value";
        CHECK_ODBC_OK(SQLExecDirect(stmt, nestedFnQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        SQLINTEGER valueInt = 0;
        SQLLEN valueInd = 0;
        ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &valueInt, 0, &valueInd), SQL_SUCCESS);
        ASSERT_EQ(valueInt, 10);
        ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    }

    {
        SQLCHAR asciiLowerQuery[] = "SELECT {fn String::AsciiToLower('AbC')} AS value";
        CHECK_ODBC_OK(SQLExecDirect(stmt, asciiLowerQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        char buf[32] = {};
        SQLLEN valueInd = 0;
        ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_CHAR, buf, sizeof(buf), &valueInd), SQL_SUCCESS);
        ASSERT_STREQ(buf, "abc");
        ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    }

    {
        SQLCHAR dateQuery[] = "SELECT {d '2024-06-15'} AS value";
        CHECK_ODBC_OK(SQLExecDirect(stmt, dateQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        char buf[32] = {};
        SQLLEN valueInd = 0;
        ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_CHAR, buf, sizeof(buf), &valueInd), SQL_SUCCESS);
        ASSERT_STREQ(buf, "2024-06-15");
        ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    }

    {
        SQLCHAR tsQuery[] = "SELECT {ts '2024-06-15 14:30:00'} AS value";
        CHECK_ODBC_OK(SQLExecDirect(stmt, tsQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        char buf[64] = {};
        SQLLEN valueInd = 0;
        ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_CHAR, buf, sizeof(buf), &valueInd), SQL_SUCCESS);
        ASSERT_STREQ(buf, "2024-06-15 14:30:00");
        ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcStmtAttr, MetadataIdSqlLikeForTableNames) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLCHAR ddl[] = R"(
        DROP TABLE IF EXISTS test_odbc_meta_like_a;
        DROP TABLE IF EXISTS test_odbc_meta_like_b;
        CREATE TABLE test_odbc_meta_like_a (id Int32, PRIMARY KEY (id));
        CREATE TABLE test_odbc_meta_like_b (id Int32, PRIMARY KEY (id));
    )";
    CHECK_ODBC_OK(SQLExecDirect(stmt, ddl, SQL_NTS), stmt, SQL_HANDLE_STMT);

    SQLULEN metadataId = SQL_TRUE;
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_METADATA_ID, &metadataId, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(metadataId, SQL_FALSE);

    const char* likePattern = "%/test_odbc_meta_like_%";
    CHECK_ODBC_OK(
        SQLTables(stmt, nullptr, 0, nullptr, 0, (SQLCHAR*)likePattern, SQL_NTS, (SQLCHAR*)"TABLE", SQL_NTS),
        stmt,
        SQL_HANDLE_STMT);
    int tableRows = 0;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        ++tableRows;
    }
    ASSERT_EQ(tableRows, 2);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    CHECK_ODBC_OK(
        SQLSetStmtAttr(stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_TRUE, 0),
        stmt,
        SQL_HANDLE_STMT);
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_METADATA_ID, &metadataId, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(metadataId, SQL_TRUE);

    ASSERT_EQ(
        SQLTables(stmt, nullptr, 0, nullptr, 0, (SQLCHAR*)likePattern, SQL_NTS, (SQLCHAR*)"TABLE", SQL_NTS),
        SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "HYC00"));
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    const std::string exactPath = "/local/test_odbc_meta_like_a";
    CHECK_ODBC_OK(
        SQLTables(stmt, nullptr, 0, nullptr, 0, (SQLCHAR*)exactPath.c_str(), SQL_NTS, (SQLCHAR*)"TABLE", SQL_NTS),
        stmt,
        SQL_HANDLE_STMT);
    tableRows = 0;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        ++tableRows;
    }
    ASSERT_EQ(tableRows, 1);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    CHECK_ODBC_OK(
        SQLSetStmtAttr(stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_FALSE, 0),
        stmt,
        SQL_HANDLE_STMT);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcStmtAttr, MetadataIdSqlLikeForColumnNames) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLCHAR ddl[] = R"(
        DROP TABLE IF EXISTS test_odbc_meta_col;
        CREATE TABLE test_odbc_meta_col (id Int32, value_x Int32, PRIMARY KEY (id));
    )";
    CHECK_ODBC_OK(SQLExecDirect(stmt, ddl, SQL_NTS), stmt, SQL_HANDLE_STMT);

    constexpr SQLUSMALLINT kColumnNameCol = 4;
    char colName[256] = {};
    SQLLEN colInd = 0;
    const std::string exactTable = "/local/test_odbc_meta_col";

    {
        CHECK_ODBC_OK(
            SQLColumns(
                stmt,
                nullptr,
                0,
                nullptr,
                0,
                (SQLCHAR*)"%/test_odbc_meta_col",
                SQL_NTS,
                (SQLCHAR*)"val%",
                SQL_NTS),
            stmt,
            SQL_HANDLE_STMT);
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        ASSERT_EQ(SQLGetData(stmt, kColumnNameCol, SQL_C_CHAR, colName, sizeof(colName), &colInd), SQL_SUCCESS);
        ASSERT_STREQ(colName, "value_x");
        ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
        ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    }

    {
        CHECK_ODBC_OK(
            SQLSetStmtAttr(stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_TRUE, 0),
            stmt,
            SQL_HANDLE_STMT);

        ASSERT_EQ(
            SQLColumns(
                stmt,
                nullptr,
                0,
                nullptr,
                0,
                (SQLCHAR*)"%/test_odbc_meta_col",
                SQL_NTS,
                (SQLCHAR*)"value_x",
                SQL_NTS),
            SQL_ERROR);
        EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "HYC00"));
        ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    }

    {
        ASSERT_EQ(
            SQLColumns(
                stmt,
                nullptr,
                0,
                nullptr,
                0,
                (SQLCHAR*)exactTable.c_str(),
                SQL_NTS,
                (SQLCHAR*)"val%",
                SQL_NTS),
            SQL_ERROR);
        EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "42S22"));
        ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

