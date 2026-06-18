#include "test_utils.h"

#include <cmath>

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
    
    CHECK_ODBC_OK(SQLExecDirect(stmt, 
        (SQLCHAR*)"SELECT * FROM AS_TABLE(ListMap(ListFromRange(1, 4), ($x) -> (AsStruct($x AS v))))",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    
    SQLLEN rowCount;
    CHECK_ODBC_OK(SQLRowCount(stmt, &rowCount), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(rowCount, -1);
    
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
