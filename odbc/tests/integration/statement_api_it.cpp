#include "test_utils.h"

#include <cmath>
#include <cstring>
#include <limits>

#ifndef SQL_ATTR_METADATA_ID
#define SQL_ATTR_METADATA_ID 10029
#endif

TEST(StatementApi, AllocFreeStmtHandle) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_STMT, stmt), SQL_SUCCESS);
    
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, ExecDirectSimple) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1 AS value", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, ExecDirectMultipleColumns) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLExecDirect(stmt, 
        (SQLCHAR*)"SELECT 1 AS int_col, 'hello' AS str_col, CAST(3.14 AS Double) AS float_col",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, ExecDirectInvalidSyntax) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    ASSERT_EQ(SQLExecDirect(stmt, (SQLCHAR*)"INVALID SYNTAX HERE", SQL_NTS), SQL_ERROR);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, ExecDirectInvalidTable) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    ASSERT_EQ(SQLExecDirect(stmt, (SQLCHAR*)"SELECT * FROM nonexistent_table", SQL_NTS), SQL_ERROR);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, PrepareAndExecuteWithQuestionMarks) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    CHECK_ODBC_OK(SQLPrepare(stmt, (SQLCHAR*)"SELECT ? + ? AS result", SQL_NTS), stmt, SQL_HANDLE_STMT);

    SQLINTEGER p1 = 10, p2 = 20;
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                                   0, 0, &p1, 0, nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                                   0, 0, &p2, 0, nullptr), stmt, SQL_HANDLE_STMT);

    CHECK_ODBC_OK(SQLExecute(stmt), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);

    SQLINTEGER result = 0;
    SQLLEN resultInd = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &result, 0, &resultInd), SQL_SUCCESS);
    ASSERT_EQ(result, 30);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, PrepareAndExecute) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLPrepare(stmt, (SQLCHAR*)"SELECT $p1 + $p2 AS result", SQL_NTS), stmt, SQL_HANDLE_STMT);
    
    SQLINTEGER p1 = 10, p2 = 20;
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 
                                   0, 0, &p1, 0, nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                                   0, 0, &p2, 0, nullptr), stmt, SQL_HANDLE_STMT);
    
    CHECK_ODBC_OK(SQLExecute(stmt), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, PrepareAndExecuteReused) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLPrepare(stmt, (SQLCHAR*)"SELECT $p1", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLINTEGER param;
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &param, 0, nullptr);
    param = 100;
    CHECK_ODBC_OK(SQLExecute(stmt), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER result;
    SQLGetData(stmt, 1, SQL_C_LONG, &result, 0, nullptr);
    ASSERT_EQ(result, 100);
    SQLCloseCursor(stmt);
    param = 200;
    CHECK_ODBC_OK(SQLExecute(stmt), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLGetData(stmt, 1, SQL_C_LONG, &result, 0, nullptr);
    ASSERT_EQ(result, 200);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, FetchSingleRow) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 42", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, FetchMultipleRows) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, 
        (SQLCHAR*)"SELECT * FROM AS_TABLE(ListMap(ListFromRange(1, 4), ($x) -> (AsStruct($x AS a)))) ORDER BY a",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLINTEGER value;
    SQLLEN ind;
    SQLBindCol(stmt, 1, SQL_C_LONG, &value, 0, &ind);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(value, 1);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(value, 2);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(value, 3);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, BindColMultipleTypes) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 42 AS col1, 'test' AS col2", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    
    SQLINTEGER col1;
    char col2[64];
    SQLLEN col1Ind, col2Ind;
    
    SQLBindCol(stmt, 1, SQL_C_LONG, &col1, 0, &col1Ind);
    SQLBindCol(stmt, 2, SQL_C_CHAR, col2, sizeof(col2), &col2Ind);
    
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(col1, 42);
    ASSERT_STREQ(col2, "test");
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, BindColThenGetData) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 100", SQL_NTS), stmt, SQL_HANDLE_STMT);
    
    SQLINTEGER value;
    SQLLEN ind;
    SQLBindCol(stmt, 1, SQL_C_LONG, &value, 0, &ind);
    
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(value, 100);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, GetDataWithoutBindCol) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 100", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    
    SQLINTEGER value;
    SQLLEN ind;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &value, 0, &ind), SQL_SUCCESS);
    ASSERT_EQ(value, 100);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, GetDataMultipleColumns) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1, 'hello world'", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    
    SQLINTEGER col1;
    SQLLEN col1Ind;
    SQLGetData(stmt, 1, SQL_C_LONG, &col1, 0, &col1Ind);
    ASSERT_EQ(col1, 1);
    
    char col2[64];
    SQLLEN col2Ind;
    SQLGetData(stmt, 2, SQL_C_CHAR, col2, sizeof(col2), &col2Ind);
    ASSERT_STREQ(col2, "hello world");
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, CloseCursor) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLCloseCursor(stmt), stmt, SQL_HANDLE_STMT);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, FreeStmtClose) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1", SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLFreeStmt(stmt, SQL_CLOSE), stmt, SQL_HANDLE_STMT);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, FreeStmtResetParams) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    SQLINTEGER param = 42;
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &param, 0, nullptr);
    
    CHECK_ODBC_OK(SQLFreeStmt(stmt, SQL_RESET_PARAMS), stmt, SQL_HANDLE_STMT);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, NumResultCols) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1, 2, 3, 4, 5", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLSMALLINT numCols;
    CHECK_ODBC_OK(SQLNumResultCols(stmt, &numCols), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(numCols, 5);
    SQLFetch(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, RowCount) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS row_count_test", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE row_count_test (id Int32, value Int32, PRIMARY KEY (id))",
        SQL_NTS), stmt, SQL_HANDLE_STMT);

    SQLLEN rowCount = -2;
    CHECK_ODBC_OK(SQLRowCount(stmt, &rowCount), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(rowCount, -1);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"UPSERT INTO row_count_test (id, value) VALUES (1, 10), (2, 20), (3, 30)",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLLEN diagRowCount = -2;
    CHECK_ODBC_OK(SQLGetDiagField(SQL_HANDLE_STMT, stmt, 0, SQL_DIAG_ROW_COUNT,
        &diagRowCount, 0, nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLRowCount(stmt, &rowCount), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(rowCount, 3);
    EXPECT_EQ(diagRowCount, rowCount);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"UPDATE row_count_test SET value = value + 1 WHERE id <= 2",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLRowCount(stmt, &rowCount), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(rowCount, 2);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"DELETE FROM row_count_test WHERE id = 3",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLRowCount(stmt, &rowCount), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(rowCount, 1);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"UPDATE row_count_test SET value = 0 WHERE id = 100",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLRowCount(stmt, &rowCount), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(rowCount, 0);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"PRAGMA TablePathPrefix = \"/local\";\n"
                  "UPDATE row_count_test SET value = value + 1 WHERE id = 1",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLRowCount(stmt, &rowCount), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(rowCount, 1);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLPrepare(stmt,
        (SQLCHAR*)"DECLARE $p1 AS Int32?;\n"
                  "UPDATE row_count_test SET value = value + 1 WHERE id = $p1",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLINTEGER nativeId = 2;
    SQLLEN nativeIdLength = 0;
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
        0, 0, &nativeId, 0, &nativeIdLength), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecute(stmt), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLRowCount(stmt, &rowCount), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(rowCount, 1);
    SQLFreeStmt(stmt, SQL_RESET_PARAMS);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"SELECT * FROM row_count_test",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLRowCount(stmt, &rowCount), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(rowCount, -1);

    SQLFreeStmt(stmt, SQL_CLOSE);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE row_count_test", SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, RowCountAggregatesParameterArrays) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS row_count_param_test", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE row_count_param_test (id Int32, value Int32, PRIMARY KEY (id))",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLPrepare(stmt,
        (SQLCHAR*)"UPSERT INTO row_count_param_test (id, value) VALUES (?, ?)",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLINTEGER ids[] = {1, 2, 3};
    SQLINTEGER values[] = {10, 20, 30};
    SQLLEN idLengths[] = {0, 0, 0};
    SQLLEN valueLengths[] = {0, 0, 0};
    SQLUSMALLINT operations[] = {SQL_PARAM_PROCEED, SQL_PARAM_IGNORE, SQL_PARAM_PROCEED};
    SQLUSMALLINT statuses[] = {SQL_PARAM_UNUSED, SQL_PARAM_UNUSED, SQL_PARAM_UNUSED};
    SQLULEN processed = 0;

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_PARAMSET_SIZE,
        reinterpret_cast<SQLPOINTER>(3), 0), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_PARAM_OPERATION_PTR,
        operations, 0), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_PARAM_STATUS_PTR,
        statuses, 0), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_PARAMS_PROCESSED_PTR,
        &processed, 0), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
        0, 0, ids, 0, idLengths), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
        0, 0, values, 0, valueLengths), stmt, SQL_HANDLE_STMT);

    CHECK_ODBC_OK(SQLExecute(stmt), stmt, SQL_HANDLE_STMT);
    SQLLEN rowCount = -1;
    CHECK_ODBC_OK(SQLRowCount(stmt, &rowCount), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(rowCount, 2);
    EXPECT_EQ(processed, 3);
    EXPECT_EQ(statuses[0], SQL_PARAM_SUCCESS);
    EXPECT_EQ(statuses[1], SQL_PARAM_UNUSED);
    EXPECT_EQ(statuses[2], SQL_PARAM_SUCCESS);

    SQLFreeStmt(stmt, SQL_CLOSE);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE row_count_param_test", SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, AttrQueryTimeout) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    SQLUINTEGER timeoutSec = 1;
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)(uintptr_t)timeoutSec, 0),
                  stmt, SQL_HANDLE_STMT);
    
    SQLCHAR longQuery[] =
        "SELECT COUNT(*) FROM AS_TABLE(ListMap(ListFromRange(1u, 100000000u), ($x)->(AsStruct($x AS v))))";
    ASSERT_EQ(SQLExecDirect(stmt, longQuery, SQL_NTS), SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "HYT00"));
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, AttrMaxRows) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_max_rows", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"CREATE TABLE test_max_rows (id Int32, PRIMARY KEY (id))", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"UPSERT INTO test_max_rows (id) VALUES (1), (2)", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    const SQLULEN maxRows = 1;
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_MAX_ROWS, (SQLPOINTER)(uintptr_t)maxRows, 0),
                  stmt, SQL_HANDLE_STMT);
    SQLULEN maxRowsOut;
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_MAX_ROWS, &maxRowsOut, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(maxRowsOut, maxRows);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT id FROM test_max_rows ORDER BY id", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, AttrNoScan) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLCHAR selectEscapeFnQuery[] = "SELECT {fn ABS(-12)} AS value";
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_OFF, 0),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, selectEscapeFnQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER valueInt = 0;
    SQLLEN valueInd = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &valueInt, 0, &valueInd), SQL_SUCCESS);
    ASSERT_EQ(valueInt, 12);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_ON, 0),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLExecDirect(stmt, selectEscapeFnQuery, SQL_NTS), SQL_ERROR);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, EscapeSequenceConvert) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_OFF, 0),
                  stmt, SQL_HANDLE_STMT);
    
    SQLCHAR convertQuery[] = "SELECT {fn CONVERT(42, SQL_SMALLINT)} AS value";
    CHECK_ODBC_OK(SQLExecDirect(stmt, convertQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLSMALLINT valueSmall = 0;
    SQLLEN valueInd = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_SSHORT, &valueSmall, 0, &valueInd), SQL_SUCCESS);
    ASSERT_EQ(valueSmall, 42);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, EscapeSequenceDouble) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_OFF, 0),
                  stmt, SQL_HANDLE_STMT);
    
    SQLCHAR convertDoubleQuery[] = "SELECT {fn CONVERT(2.5, SQL_DOUBLE)} AS value";
    CHECK_ODBC_OK(SQLExecDirect(stmt, convertDoubleQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    double valueDouble = 0;
    SQLLEN valueInd = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_DOUBLE, &valueDouble, 0, &valueInd), SQL_SUCCESS);
    ASSERT_LT(std::fabs(valueDouble - 2.5), 1e-9);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, EscapeSequenceNested) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_OFF, 0),
                  stmt, SQL_HANDLE_STMT);
    
    SQLCHAR nestedFnQuery[] = "SELECT {fn {fn ABS(-10)}} AS value";
    CHECK_ODBC_OK(SQLExecDirect(stmt, nestedFnQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER valueInt = 0;
    SQLLEN valueInd = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &valueInt, 0, &valueInd), SQL_SUCCESS);
    ASSERT_EQ(valueInt, 10);
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, EscapeSequenceString) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_OFF, 0),
                  stmt, SQL_HANDLE_STMT);
    
    SQLCHAR asciiLowerQuery[] = "SELECT {fn String::AsciiToLower('AbC')} AS value";
    CHECK_ODBC_OK(SQLExecDirect(stmt, asciiLowerQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    char buf[32] = {};
    SQLLEN valueInd = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_CHAR, buf, sizeof(buf), &valueInd), SQL_SUCCESS);
    ASSERT_STREQ(buf, "abc");
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, EscapeSequenceDate) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_OFF, 0),
                  stmt, SQL_HANDLE_STMT);
    
    SQLCHAR dateQuery[] = "SELECT {d '2024-06-15'} AS value";
    CHECK_ODBC_OK(SQLExecDirect(stmt, dateQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    char buf[32] = {};
    SQLLEN valueInd = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_CHAR, buf, sizeof(buf), &valueInd), SQL_SUCCESS);
    ASSERT_STREQ(buf, "2024-06-15");
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, EscapeSequenceTimestamp) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_OFF, 0),
                  stmt, SQL_HANDLE_STMT);
    
    SQLCHAR tsQuery[] = "SELECT {ts '2024-06-15 14:30:00'} AS value";
    CHECK_ODBC_OK(SQLExecDirect(stmt, tsQuery, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    char buf[64] = {};
    SQLLEN valueInd = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_CHAR, buf, sizeof(buf), &valueInd), SQL_SUCCESS);
    ASSERT_STREQ(buf, "2024-06-15 14:30:00");
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, CurrentUtcTimestampConversion) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLExecDirect(stmt,
        (SQLCHAR*)"DROP TABLE IF EXISTS test_current_timestamp_conversion", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_current_timestamp_conversion ("
                  "id Int32, value Timestamp, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"INSERT INTO test_current_timestamp_conversion (id, value) "
                  "VALUES (1, CurrentUtcTimestamp())", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"SELECT id, value FROM test_current_timestamp_conversion WHERE id = 1", SQL_NTS),
        stmt, SQL_HANDLE_STMT);

    SQLCHAR columnName[32] = {};
    SQLSMALLINT nameLength = 0;
    SQLSMALLINT dataType = 0;
    SQLULEN columnSize = 0;
    SQLSMALLINT decimalDigits = 0;
    SQLSMALLINT nullable = 0;
    CHECK_ODBC_OK(SQLDescribeCol(stmt, 2, columnName, sizeof(columnName), &nameLength,
                                &dataType, &columnSize, &decimalDigits, &nullable),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(dataType, SQL_TYPE_TIMESTAMP);

    SQL_TIMESTAMP_STRUCT value{};
    SQLLEN indicator = 0;
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLGetData(stmt, 2, SQL_C_TIMESTAMP, &value, 0, &indicator),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(indicator, static_cast<SQLLEN>(sizeof(value)));
    EXPECT_GE(value.year, 2026);
    EXPECT_GE(value.month, 1);
    EXPECT_LE(value.month, 12);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, NumericOutOfRange) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"SELECT CAST(3000000000 AS Uint64) AS v", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER value = 0;
    SQLLEN indicator = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &value, sizeof(value), &indicator), SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "22003"));
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, UpsertAutocommitPersist) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_upsert_persist", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_upsert_persist (id Int32, val Int32, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"UPSERT INTO test_upsert_persist (id, val) VALUES (1, 42)", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"SELECT val FROM test_upsert_persist WHERE id = 1", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER val = 0;
    SQLLEN ind = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &val, sizeof(val), &ind), SQL_SUCCESS);
    ASSERT_EQ(val, 42);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, SqlCBit) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT true AS b", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    char bitVal = 0;
    SQLLEN ind = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_BIT, &bitVal, sizeof(bitVal), &ind), SQL_SUCCESS);
    ASSERT_EQ(bitVal, 1);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT false AS b", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_BIT, &bitVal, sizeof(bitVal), &ind), SQL_SUCCESS);
    ASSERT_EQ(bitVal, 0);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, CoreBoundTypeConversions) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_STATIC, 0),
                  stmt, SQL_HANDLE_STMT);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_bound_types", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    // Keep this server-backed test on YDB's default decimal shape, which does
    // not require support for non-default parameterized decimals.
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_bound_types ("
                  "id Int32, u64 Uint64, text Utf8, bytes String, d Date, "
                  "tm Datetime, ts Timestamp, dec Decimal(22, 9), PRIMARY KEY (id))",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLPrepare(stmt,
        (SQLCHAR*)"INSERT INTO test_bound_types "
                  "(id, u64, text, bytes, d, tm, ts, dec) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
        SQL_NTS), stmt, SQL_HANDLE_STMT);

    SQLINTEGER id = 1;
    SQLUBIGINT u64 = std::numeric_limits<SQLUBIGINT>::max();
    SQLWCHAR text[] = {'Y', 'D', 'B', 0};
    char bytes[] = {'a', '\0', 'b', 'c'};
    SQL_DATE_STRUCT date{2024, 6, 15};
    SQL_TIME_STRUCT time{14, 30, 20};
    SQL_TIMESTAMP_STRUCT timestamp{2024, 6, 15, 14, 30, 20, 123456000};
    SQLDOUBLE decimal = 123.456;
    SQLLEN textLength = SQL_NTS;
    SQLLEN bytesLength = sizeof(bytes);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
                                   0, 0, &id, 0, nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_UBIGINT, SQL_BIGINT,
                                   0, 0, &u64, 0, nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,
                                   3, 0, text, sizeof(text), &textLength), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_VARBINARY,
                                   sizeof(bytes), 0, bytes, sizeof(bytes), &bytesLength), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_TYPE_DATE,
                                   0, 0, &date, 0, nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 6, SQL_PARAM_INPUT, SQL_C_TYPE_TIME, SQL_TYPE_TIME,
                                   0, 0, &time, 0, nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 7, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
                                   0, 0, &timestamp, 0, nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(stmt, 8, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL,
                                   0, 0, &decimal, 0, nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecute(stmt), stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_RESET_PARAMS);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"SELECT u64, text, bytes, d, tm, ts, dec, dec AS dec_text "
                  "FROM test_bound_types WHERE id = 1",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLCHAR columnName[32] = {};
    SQLSMALLINT nameLength = 0;
    SQLSMALLINT describedType = 0;
    SQLULEN columnSize = 0;
    SQLSMALLINT decimalDigits = 0;
    SQLSMALLINT nullable = 0;
    CHECK_ODBC_OK(SQLDescribeCol(stmt, 5, columnName, sizeof(columnName), &nameLength,
                                &describedType, &columnSize, &decimalDigits, &nullable),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(describedType, SQL_TYPE_TIME);
    CHECK_ODBC_OK(SQLDescribeCol(stmt, 6, columnName, sizeof(columnName), &nameLength,
                                &describedType, &columnSize, &decimalDigits, &nullable),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(describedType, SQL_TYPE_TIMESTAMP);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLUBIGINT fetchedU64 = 0;
    SQLWCHAR fetchedText[4] = {};
    char fetchedBytes[4] = {};
    SQL_DATE_STRUCT fetchedDate{};
    SQL_TIME_STRUCT fetchedTime{};
    SQL_TIMESTAMP_STRUCT fetchedTimestamp{};
    SQLDOUBLE fetchedDecimal = 0;
    char fetchedDecimalText[32] = {};
    SQLLEN indicator = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_UBIGINT, &fetchedU64, sizeof(fetchedU64), &indicator), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 2, SQL_C_WCHAR, fetchedText, sizeof(fetchedText), &indicator), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 3, SQL_C_BINARY, fetchedBytes, sizeof(fetchedBytes), &indicator), SQL_SUCCESS);
    EXPECT_EQ(indicator, static_cast<SQLLEN>(sizeof(bytes)));
    ASSERT_EQ(SQLGetData(stmt, 4, SQL_C_TYPE_DATE, &fetchedDate, sizeof(fetchedDate), &indicator), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 5, SQL_C_TYPE_TIME, &fetchedTime, sizeof(fetchedTime), &indicator), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 6, SQL_C_TYPE_TIMESTAMP, &fetchedTimestamp, sizeof(fetchedTimestamp), &indicator), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 7, SQL_C_DOUBLE, &fetchedDecimal, sizeof(fetchedDecimal), &indicator), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 8, SQL_C_CHAR, fetchedDecimalText, sizeof(fetchedDecimalText), &indicator), SQL_SUCCESS);

    EXPECT_EQ(fetchedU64, u64);
    EXPECT_EQ(fetchedText[0], 'Y');
    EXPECT_EQ(fetchedText[1], 'D');
    EXPECT_EQ(fetchedText[2], 'B');
    EXPECT_EQ(std::memcmp(fetchedBytes, bytes, sizeof(bytes)), 0);
    EXPECT_EQ(fetchedDate.year, date.year);
    EXPECT_EQ(fetchedDate.month, date.month);
    EXPECT_EQ(fetchedDate.day, date.day);
    EXPECT_EQ(fetchedTime.hour, time.hour);
    EXPECT_EQ(fetchedTime.minute, time.minute);
    EXPECT_EQ(fetchedTime.second, time.second);
    EXPECT_EQ(fetchedTimestamp.fraction, timestamp.fraction);
    EXPECT_NEAR(fetchedDecimal, decimal, 1e-9);
    EXPECT_STREQ(fetchedDecimalText, "123.456000000");

    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_FIRST, 0), SQL_SUCCESS);
    std::memset(fetchedBytes, 0, sizeof(fetchedBytes));
    fetchedDecimal = 0;
    ASSERT_EQ(SQLGetData(stmt, 3, SQL_C_BINARY, fetchedBytes, sizeof(fetchedBytes), &indicator), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 7, SQL_C_DOUBLE, &fetchedDecimal, sizeof(fetchedDecimal), &indicator), SQL_SUCCESS);
    EXPECT_EQ(std::memcmp(fetchedBytes, bytes, sizeof(bytes)), 0);
    EXPECT_NEAR(fetchedDecimal, decimal, 1e-9);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, CursorAttributesAndCapabilities) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLULEN value = 0;
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE, &value, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(value, SQL_CURSOR_FORWARD_ONLY);
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_CURSOR_SCROLLABLE, &value, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(value, SQL_NONSCROLLABLE);
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_CONCURRENCY, &value, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(value, SQL_CONCUR_READ_ONLY);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_CONCURRENCY,
                                (SQLPOINTER)SQL_CONCUR_READ_ONLY, 0),
                  stmt, SQL_HANDLE_STMT);

    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(SQLFetchScroll(stmt, SQL_FETCH_PRIOR, 0), SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "HY106"));
    CHECK_ODBC_OK(SQLFreeStmt(stmt, SQL_CLOSE), stmt, SQL_HANDLE_STMT);

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_SCROLLABLE,
                                (SQLPOINTER)SQL_SCROLLABLE, 0),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_CURSOR_SCROLLABLE, &value, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(value, SQL_SCROLLABLE);
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE, &value, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(value, SQL_CURSOR_STATIC);

    EXPECT_EQ(SQLSetStmtAttr(stmt, SQL_ATTR_CONCURRENCY,
                             (SQLPOINTER)SQL_CONCUR_VALUES, 0),
              SQL_SUCCESS_WITH_INFO);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "01S02"));
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_CONCURRENCY, &value, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(value, SQL_CONCUR_READ_ONLY);

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE,
                             (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0),
              SQL_SUCCESS_WITH_INFO);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "01S02"));
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE, &value, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(value, SQL_CURSOR_STATIC);

    SQLUINTEGER info = 0;
    CHECK_ODBC_OK(SQLGetInfo(dbc, SQL_STATIC_CURSOR_ATTRIBUTES1, &info, 0, nullptr),
                  dbc, SQL_HANDLE_DBC);
    EXPECT_EQ(info & (SQL_CA1_NEXT | SQL_CA1_ABSOLUTE | SQL_CA1_RELATIVE),
              SQL_CA1_NEXT | SQL_CA1_ABSOLUTE | SQL_CA1_RELATIVE);
    CHECK_ODBC_OK(SQLGetInfo(dbc, SQL_SCROLL_OPTIONS, &info, 0, nullptr),
                  dbc, SQL_HANDLE_DBC);
    EXPECT_EQ(info & (SQL_SO_FORWARD_ONLY | SQL_SO_STATIC),
              SQL_SO_FORWARD_ONLY | SQL_SO_STATIC);
    CHECK_ODBC_OK(SQLGetInfo(dbc, SQL_SCROLL_CONCURRENCY, &info, 0, nullptr),
                  dbc, SQL_HANDLE_DBC);
    EXPECT_EQ(info & SQL_SCCO_READ_ONLY, SQL_SCCO_READ_ONLY);
    CHECK_ODBC_OK(SQLGetInfo(dbc, SQL_STATIC_CURSOR_ATTRIBUTES2, &info, 0, nullptr),
                  dbc, SQL_HANDLE_DBC);
    EXPECT_EQ(info & SQL_CA2_READ_ONLY_CONCURRENCY, SQL_CA2_READ_ONLY_CONCURRENCY);

    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE,
                             (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0),
              SQL_ERROR);
    const std::string cursorTypeError = GetOdbcError(stmt, SQL_HANDLE_STMT);
    EXPECT_TRUE(SqlStatePrefix(cursorTypeError, "24000")) << cursorTypeError;
    EXPECT_EQ(SQLSetStmtAttr(stmt, SQL_ATTR_CONCURRENCY,
                             (SQLPOINTER)SQL_CONCUR_READ_ONLY, 0),
              SQL_ERROR);
    const std::string concurrencyError = GetOdbcError(stmt, SQL_HANDLE_STMT);
    EXPECT_TRUE(SqlStatePrefix(concurrencyError, "24000")) << concurrencyError;
    CHECK_ODBC_OK(SQLFreeStmt(stmt, SQL_CLOSE), stmt, SQL_HANDLE_STMT);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, StaticCursorScrollsRowsets) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_STATIC, 0),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                (SQLPOINTER)(uintptr_t)3, 0),
                  stmt, SQL_HANDLE_STMT);

    SQLINTEGER values[3] = {};
    SQLLEN indicators[3] = {};
    SQLUSMALLINT statuses[3] = {};
    SQLULEN fetched = 0;
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, statuses, 0),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR, &fetched, 0),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindCol(stmt, 1, SQL_C_LONG, values, 0, indicators),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"SELECT * FROM AS_TABLE(ListMap(ListFromRange(1, 8), "
                  "($x) -> (AsStruct($x AS value)))) ORDER BY value",
        SQL_NTS), stmt, SQL_HANDLE_STMT);

    SQLULEN rowNumber = 0;
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0), SQL_SUCCESS);
    EXPECT_EQ(fetched, 3);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
    EXPECT_EQ(values[2], 3);
    EXPECT_EQ(statuses[0], SQL_ROW_SUCCESS);
    EXPECT_EQ(statuses[1], SQL_ROW_SUCCESS);
    EXPECT_EQ(statuses[2], SQL_ROW_SUCCESS);
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNumber, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(rowNumber, 1);
    SQLINTEGER current = 0;
    SQLLEN currentIndicator = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &current, 0, &currentIndicator), SQL_SUCCESS);
    EXPECT_EQ(current, 1);

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                (SQLPOINTER)(uintptr_t)2, 0),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0), SQL_SUCCESS);
    EXPECT_EQ(fetched, 2);
    EXPECT_EQ(values[0], 4);
    EXPECT_EQ(values[1], 5);
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNumber, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(rowNumber, 4);

    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_PRIOR, 0), SQL_SUCCESS);
    EXPECT_EQ(values[0], 2);
    EXPECT_EQ(values[1], 3);

    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_FIRST, 0), SQL_SUCCESS);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_RELATIVE, 3), SQL_SUCCESS);
    EXPECT_EQ(values[0], 4);
    EXPECT_EQ(values[1], 5);

    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_LAST, 0), SQL_SUCCESS);
    EXPECT_EQ(values[0], 6);
    EXPECT_EQ(values[1], 7);
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNumber, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(rowNumber, 6);
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_ABSOLUTE, -3), SQL_SUCCESS);
    EXPECT_EQ(values[0], 5);
    EXPECT_EQ(values[1], 6);

    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_ABSOLUTE, 0), SQL_NO_DATA);
    EXPECT_EQ(fetched, 0);
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0), SQL_SUCCESS);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);

    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_ABSOLUTE, 100), SQL_NO_DATA);
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_PRIOR, 0), SQL_SUCCESS);
    EXPECT_EQ(values[0], 6);
    EXPECT_EQ(values[1], 7);

    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_ABSOLUTE, 2), SQL_SUCCESS);
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_PRIOR, 0), SQL_SUCCESS_WITH_INFO);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "01S06"));
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);

    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_ABSOLUTE, 6), SQL_SUCCESS);
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_RELATIVE, -6), SQL_SUCCESS_WITH_INFO);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "01S06"));
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);

    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_ABSOLUTE, -1), SQL_SUCCESS);
    EXPECT_EQ(fetched, 1);
    EXPECT_EQ(values[0], 7);
    EXPECT_EQ(statuses[0], SQL_ROW_SUCCESS);
    EXPECT_EQ(statuses[1], SQL_ROW_NOROW);
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_ROW_NUMBER, &rowNumber, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(rowNumber, 7);
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &current, 0, &currentIndicator), SQL_SUCCESS);
    EXPECT_EQ(current, 7);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(StatementApi, StaticCursorHonorsMaxRows) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_STATIC, 0),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_MAX_ROWS,
                                (SQLPOINTER)(uintptr_t)5, 0),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                (SQLPOINTER)(uintptr_t)3, 0),
                  stmt, SQL_HANDLE_STMT);

    SQLINTEGER values[3] = {};
    SQLLEN indicators[3] = {};
    SQLULEN fetched = 0;
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR, &fetched, 0),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindCol(stmt, 1, SQL_C_LONG, values, 0, indicators),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"SELECT * FROM AS_TABLE(ListMap(ListFromRange(1, 9), "
                  "($x) -> (AsStruct($x AS value)))) ORDER BY value",
        SQL_NTS), stmt, SQL_HANDLE_STMT);

    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_LAST, 0), SQL_SUCCESS);
    EXPECT_EQ(fetched, 3);
    EXPECT_EQ(values[0], 3);
    EXPECT_EQ(values[1], 4);
    EXPECT_EQ(values[2], 5);
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_ABSOLUTE, 6), SQL_NO_DATA);
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_FIRST, 0), SQL_SUCCESS);
    ASSERT_EQ(SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0), SQL_SUCCESS);
    EXPECT_EQ(fetched, 2);
    EXPECT_EQ(values[0], 4);
    EXPECT_EQ(values[1], 5);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}
