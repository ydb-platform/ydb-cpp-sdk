#include "test_utils.h"

#ifndef SQL_ATTR_METADATA_ID
#define SQL_ATTR_METADATA_ID 10029
#endif

TEST(MetadataApi, SQLTablesAll) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLTables(stmt, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0),
                  stmt, SQL_HANDLE_STMT);
    int rowCount = 0;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        ++rowCount;
    }
    ASSERT_GT(rowCount, 0);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(MetadataApi, SQLTablesWithPattern) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_metadata_pattern_a", SQL_NTS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_metadata_pattern_b", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_metadata_pattern_a (id Int32, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_metadata_pattern_b (id Int32, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLTables(stmt, nullptr, 0, nullptr, 0,
                           (SQLCHAR*)"%/test_metadata_pattern_%", SQL_NTS, nullptr, 0),
                  stmt, SQL_HANDLE_STMT);
    int tableCount = 0;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        ++tableCount;
    }
    ASSERT_EQ(tableCount, 2);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(MetadataApi, SQLTablesExactMatch) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_exact_table", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_exact_table (id Int32, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_TRUE, 0),
                  stmt, SQL_HANDLE_STMT);
    const std::string exactPath = "/local/test_exact_table";
    CHECK_ODBC_OK(SQLTables(stmt, nullptr, 0, nullptr, 0,
                           (SQLCHAR*)exactPath.c_str(), SQL_NTS, nullptr, 0),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(MetadataApi, SQLTablesLikePatternWithMetadataId) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_meta_table_1", SQL_NTS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_meta_table_2", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_meta_table_1 (id Int32, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_meta_table_2 (id Int32, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    SQLULEN metadataId = SQL_FALSE;
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_METADATA_ID, &metadataId, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(metadataId, SQL_FALSE);
    const char* likePattern = "%/test_meta_table_%";
    CHECK_ODBC_OK(SQLTables(stmt, nullptr, 0, nullptr, 0,
                           (SQLCHAR*)likePattern, SQL_NTS, (SQLCHAR*)"TABLE", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    int tableRows = 0;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        ++tableRows;
    }
    ASSERT_EQ(tableRows, 2);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_TRUE, 0),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLGetStmtAttr(stmt, SQL_ATTR_METADATA_ID, &metadataId, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(metadataId, SQL_TRUE);
    ASSERT_EQ(SQLTables(stmt, nullptr, 0, nullptr, 0,
                       (SQLCHAR*)likePattern, SQL_NTS, (SQLCHAR*)"TABLE", SQL_NTS),
              SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "HYC00"));
    SQLFreeStmt(stmt, SQL_CLOSE);
    const std::string exactPath = "/local/test_meta_table_1";
    CHECK_ODBC_OK(SQLTables(stmt, nullptr, 0, nullptr, 0,
                           (SQLCHAR*)exactPath.c_str(), SQL_NTS, (SQLCHAR*)"TABLE", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    tableRows = 0;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        ++tableRows;
    }
    ASSERT_EQ(tableRows, 1);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_FALSE, 0),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(MetadataApi, SQLColumnsAll) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_columns_all", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_columns_all (id Int32, name Text, value Int32, PRIMARY KEY (id))",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLColumns(stmt, nullptr, 0, nullptr, 0,
                            (SQLCHAR*)"/local/test_columns_all", SQL_NTS, nullptr, 0),
                  stmt, SQL_HANDLE_STMT);
    int colCount = 0;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        ++colCount;
    }
    ASSERT_EQ(colCount, 3);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(MetadataApi, SQLColumnsWithPattern) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_columns_pattern", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_columns_pattern (id Int32, value_x Int32, value_y Int32, PRIMARY KEY (id))",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    constexpr SQLUSMALLINT kColumnNameCol = 4;
    char colName[256] = {};
    SQLLEN colInd = 0;
    CHECK_ODBC_OK(SQLColumns(stmt, nullptr, 0, nullptr, 0,
                            (SQLCHAR*)"/local/test_columns_pattern", SQL_NTS,
                            (SQLCHAR*)"val%", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, kColumnNameCol, SQL_C_CHAR, colName, sizeof(colName), &colInd), SQL_SUCCESS);
    ASSERT_STREQ(colName, "value_x");
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, kColumnNameCol, SQL_C_CHAR, colName, sizeof(colName), &colInd), SQL_SUCCESS);
    ASSERT_STREQ(colName, "value_y");
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(MetadataApi, SQLColumnsMetadataId) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_columns_metadata", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_columns_metadata (id Int32, value_x Int32, PRIMARY KEY (id))",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    const std::string exactTable = "/local/test_columns_metadata";
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_TRUE, 0),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLColumns(stmt, nullptr, 0, nullptr, 0,
                        (SQLCHAR*)exactTable.c_str(), SQL_NTS,
                        (SQLCHAR*)"val%", SQL_NTS),
              SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "42S22"));
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_FALSE, 0),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}
