#include "test_utils.h"

#include <cstring>

#ifndef SQL_ODBC_INTERFACE_CONFORMANCE
#define SQL_ODBC_INTERFACE_CONFORMANCE 169
#endif

TEST(CoreApi, SQLGetTypeInfoAll) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLGetTypeInfo(stmt, SQL_ALL_TYPES), stmt, SQL_HANDLE_STMT);
    char typeName[64] = {};
    SQLLEN indicator = 0;
    SQLBindCol(stmt, 1, SQL_C_CHAR, typeName, sizeof(typeName), &indicator);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    EXPECT_TRUE(std::strstr(typeName, "bigint") != nullptr);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLGetTypeInfoFilter) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLGetTypeInfo(stmt, SQL_INTEGER), stmt, SQL_HANDLE_STMT);
    SQLINTEGER dataType = 0;
    SQLLEN indicator = 0;
    SQLBindCol(stmt, 2, SQL_C_LONG, &dataType, 0, &indicator);
    int rowCount = 0;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        ASSERT_EQ(dataType, SQL_INTEGER);
        ++rowCount;
    }
    ASSERT_GT(rowCount, 0);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLNumParamsQuestionMarks) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLPrepare(stmt, (SQLCHAR*)"SELECT ? + ?", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLSMALLINT paramCount = 0;
    CHECK_ODBC_OK(SQLNumParams(stmt, &paramCount), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(paramCount, 2);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLNumParamsDollarParams) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLPrepare(stmt, (SQLCHAR*)"SELECT $p1", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLSMALLINT paramCount = 0;
    CHECK_ODBC_OK(SQLNumParams(stmt, &paramCount), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(paramCount, 1);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLColAttributeName) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1 AS col", SQL_NTS), stmt, SQL_HANDLE_STMT);
    char name[64] = {};
    SQLSMALLINT nameLen = 0;
    CHECK_ODBC_OK(SQLColAttribute(stmt, 1, SQL_DESC_NAME, name, sizeof(name), &nameLen, nullptr),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_STREQ(name, "col");
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLColAttributeType) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1 AS col", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLLEN dataType = 0;
    CHECK_ODBC_OK(SQLColAttribute(stmt, 1, SQL_DESC_TYPE, nullptr, 0, nullptr, &dataType),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(dataType, SQL_INTEGER);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLNativeSqlPassthrough) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    char out[64] = {};
    SQLINTEGER outLen = 0;
    CHECK_ODBC_OK(SQLNativeSql(dbc, (SQLCHAR*)"SELECT 1", SQL_NTS, (SQLCHAR*)out, sizeof(out), &outLen),
                  dbc, SQL_HANDLE_DBC);
    EXPECT_STREQ(out, "SELECT 1");
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLSetGetCursorName) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLSetCursorName(stmt, (SQLCHAR*)"mycursor", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLCHAR name[64] = {};
    SQLSMALLINT nameLen = 0;
    CHECK_ODBC_OK(SQLGetCursorName(stmt, name, sizeof(name), &nameLen), stmt, SQL_HANDLE_STMT);
    EXPECT_STREQ(reinterpret_cast<const char*>(name), "mycursor");
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLStatisticsEmpty) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLStatistics(stmt, nullptr, 0, nullptr, 0, (SQLCHAR*)"%", SQL_NTS, SQL_INDEX_ALL, SQL_ENSURE),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLSpecialColumnsPrimaryKey) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_special_columns_pk", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_special_columns_pk (id Int32, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    const char* table = "/local/test_special_columns_pk";
    CHECK_ODBC_OK(SQLSpecialColumns(stmt, SQL_BEST_ROWID, nullptr, 0, nullptr, 0,
                                    (SQLCHAR*)table, SQL_NTS, SQL_SCOPE_SESSION, 0),
                  stmt, SQL_HANDLE_STMT);
    char columnName[64] = {};
    SQLLEN indicator = 0;
    SQLBindCol(stmt, 2, SQL_C_CHAR, columnName, sizeof(columnName), &indicator);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    EXPECT_STREQ(columnName, "id");
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLGetInfoInterfaceConformance) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    SQLUINTEGER conformance = 0;
    SQLSMALLINT outLen = 0;
    CHECK_ODBC_OK(SQLGetInfo(dbc, SQL_ODBC_INTERFACE_CONFORMANCE, &conformance, 0, &outLen),
                  dbc, SQL_HANDLE_DBC);
    EXPECT_EQ(conformance, SQL_OIC_CORE);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLForeignKeysEmpty) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLForeignKeys(stmt, nullptr, 0, nullptr, 0, (SQLCHAR*)"%", SQL_NTS,
                                 nullptr, 0, nullptr, 0, (SQLCHAR*)"%", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLSMALLINT colCount = 0;
    CHECK_ODBC_OK(SQLNumResultCols(stmt, &colCount), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(colCount, 14);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLPrimaryKeys) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_primary_keys", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_primary_keys (id Int32, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    const char* table = "/local/test_primary_keys";
    CHECK_ODBC_OK(SQLPrimaryKeys(stmt, nullptr, 0, nullptr, 0, (SQLCHAR*)table, SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    char columnName[64] = {};
    SQLLEN indicator = 0;
    SQLBindCol(stmt, 4, SQL_C_CHAR, columnName, sizeof(columnName), &indicator);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    EXPECT_STREQ(columnName, "id");
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLDescribeParamUnknown) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLPrepare(stmt, (SQLCHAR*)"SELECT ?", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLSMALLINT dataType = 0;
    SQLULEN paramSize = 0;
    SQLSMALLINT decimalDigits = 0;
    SQLSMALLINT nullable = 0;
    CHECK_ODBC_OK(SQLDescribeParam(stmt, 1, &dataType, &paramSize, &decimalDigits, &nullable),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(dataType, SQL_UNKNOWN_TYPE);
    EXPECT_EQ(nullable, SQL_NULLABLE_UNKNOWN);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLDescribeParamBound) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLPrepare(stmt, (SQLCHAR*)"SELECT ?", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLINTEGER value = 42;
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &value, 0, nullptr),
                  stmt, SQL_HANDLE_STMT);
    SQLSMALLINT dataType = 0;
    SQLULEN paramSize = 0;
    SQLSMALLINT decimalDigits = 0;
    SQLSMALLINT nullable = 0;
    CHECK_ODBC_OK(SQLDescribeParam(stmt, 1, &dataType, &paramSize, &decimalDigits, &nullable),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(dataType, SQL_INTEGER);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLParamDataPutData) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_at_exec", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_at_exec (id Int32, val Text, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLPrepare(stmt, (SQLCHAR*)"UPSERT INTO test_at_exec (id, val) VALUES (1, ?)", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLLEN atExec = SQL_DATA_AT_EXEC;
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 32, 0,
                                   nullptr, 0, &atExec), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLExecute(stmt), SQL_NEED_DATA);
    SQLPOINTER token = nullptr;
    ASSERT_EQ(SQLParamData(stmt, &token), SQL_NEED_DATA);
    const char part1[] = "hel";
    CHECK_ODBC_OK(SQLPutData(stmt, (SQLPOINTER)part1, sizeof(part1) - 1), stmt, SQL_HANDLE_STMT);
    const char part2[] = "lo";
    CHECK_ODBC_OK(SQLPutData(stmt, (SQLPOINTER)part2, sizeof(part2) - 1), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLPutData(stmt, nullptr, 0), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLParamData(stmt, &token), stmt, SQL_HANDLE_STMT);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLCancel) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"SELECT * FROM AS_TABLE(ListMap(ListFromRange(1u, 1000000u), ($x)->(AsStruct($x AS v))))",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLCancel(stmt), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1", SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLFreeStmtDrop) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_DROP), SQL_SUCCESS);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLParamDataPutDataNts) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_at_exec_nts", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_at_exec_nts (id Int32, val Text, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLPrepare(stmt, (SQLCHAR*)"UPSERT INTO test_at_exec_nts (id, val) VALUES (2, ?)", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLLEN atExec = SQL_DATA_AT_EXEC;
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 32, 0,
                                   nullptr, 0, &atExec), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLExecute(stmt), SQL_NEED_DATA);
    SQLPOINTER token = nullptr;
    ASSERT_EQ(SQLParamData(stmt, &token), SQL_NEED_DATA);
    const char payload[] = "nts-value";
    CHECK_ODBC_OK(SQLPutData(stmt, (SQLPOINTER)payload, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLParamData(stmt, &token), stmt, SQL_HANDLE_STMT);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLCancelIdle) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLCancel(stmt), stmt, SQL_HANDLE_STMT);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}
