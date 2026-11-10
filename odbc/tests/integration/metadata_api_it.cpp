#include "test_utils.h"

#include <array>
#include <initializer_list>
#include <optional>
#include <set>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#ifndef SQL_ATTR_METADATA_ID
#define SQL_ATTR_METADATA_ID 10029
#endif

namespace {

struct TMetadataHandles {
    SQLHENV Env = SQL_NULL_HENV;
    SQLHDBC Dbc = SQL_NULL_HDBC;
    SQLHSTMT Stmt = SQL_NULL_HSTMT;

    TMetadataHandles() {
        AllocEnvAndConnect(&Env, &Dbc);
        EXPECT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, Dbc, &Stmt), SQL_SUCCESS);
    }

    ~TMetadataHandles() {
        SQLFreeHandle(SQL_HANDLE_STMT, Stmt);
        SQLDisconnect(Dbc);
        SQLFreeHandle(SQL_HANDLE_DBC, Dbc);
        SQLFreeHandle(SQL_HANDLE_ENV, Env);
    }
};

void ExecuteAndClose(SQLHSTMT stmt, const char* sql) {
    CHECK_ODBC_OK(SQLExecDirect(
        stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>(sql)), SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
}

std::optional<std::string> ReadText(SQLHSTMT stmt, SQLUSMALLINT column) {
    char value[512] = {};
    SQLLEN indicator = 0;
    const SQLRETURN rc = SQLGetData(
        stmt, column, SQL_C_CHAR, value, sizeof(value), &indicator);
    if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
        ADD_FAILURE() << GetOdbcError(stmt, SQL_HANDLE_STMT);
        return std::nullopt;
    }
    return indicator == SQL_NULL_DATA ? std::nullopt
                                      : std::optional<std::string>{value};
}

constexpr std::monostate AnyValue;
using TExpectedValue = std::variant<std::nullptr_t, std::string_view, int64_t, std::monostate>;
using TExpectedCell = std::pair<SQLUSMALLINT, TExpectedValue>;

void ExpectRow(SQLHSTMT stmt, std::initializer_list<TExpectedCell> cells) {
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    for (const auto& [column, expected] : cells) {
        const auto value = ReadText(stmt, column);
        if (std::holds_alternative<std::nullptr_t>(expected)) {
            EXPECT_FALSE(value) << "column " << column;
            continue;
        }
        ASSERT_TRUE(value) << "column " << column;
        if (const auto* text = std::get_if<std::string_view>(&expected)) {
            EXPECT_EQ(*value, *text) << "column " << column;
        } else if (const auto* number = std::get_if<int64_t>(&expected)) {
            EXPECT_EQ(*value, std::to_string(*number)) << "column " << column;
        }
    }
}

void FinishResult(SQLHSTMT stmt) {
    EXPECT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
}

void ExpectError(SQLRETURN result, SQLHSTMT stmt, std::string_view state) {
    EXPECT_EQ(result, SQL_ERROR);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), state));
}

} // namespace

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
                           (SQLCHAR*)"test\\_metadata\\_pattern\\_%", SQL_NTS, nullptr, 0),
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
    SQLCHAR catalog[] = "local";
    SQLCHAR schema[] = "";
    CHECK_ODBC_OK(SQLTables(stmt, catalog, SQL_NTS, schema, 0,
                           (SQLCHAR*)exactPath.c_str(), SQL_NTS, nullptr, 0),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(MetadataApi, RelativeTableMetadataUsesCurrentCatalog) {
    struct TExpectedColumn {
        const char* Name;
        const char* TypeName;
        SQLSMALLINT Nullable;
        const char* IsNullable;
    };
    const TExpectedColumn expectedColumns[] = {
        {"id", "Int32", SQL_NO_NULLS, "NO"},
        {"value", "Utf8", SQL_NULLABLE, "YES"},
    };

    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_relative_metadata", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_relative_metadata (id Int32 NOT NULL, value Utf8, PRIMARY KEY (id))",
        SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);

    const char* table = "test_relative_metadata";
    CHECK_ODBC_OK(SQLTables(stmt, nullptr, 0, nullptr, 0,
                           (SQLCHAR*)table, SQL_NTS, (SQLCHAR*)"TABLE", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    char catalog[64] = {};
    char tableName[64] = {};
    SQLLEN indicator = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_CHAR, catalog, sizeof(catalog), &indicator), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 3, SQL_C_CHAR, tableName, sizeof(tableName), &indicator), SQL_SUCCESS);
    EXPECT_STREQ(catalog, "local");
    EXPECT_STREQ(tableName, table);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLColumns(stmt, nullptr, 0, nullptr, 0,
                            (SQLCHAR*)table, SQL_NTS, nullptr, 0),
                  stmt, SQL_HANDLE_STMT);
    for (const auto& column : expectedColumns) {
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        char columnName[64] = {};
        char typeName[64] = {};
        char isNullable[8] = {};
        SQLSMALLINT nullable = -1;
        CHECK_ODBC_OK(SQLGetData(stmt, 4, SQL_C_CHAR, columnName,
                                sizeof(columnName), &indicator),
                      stmt, SQL_HANDLE_STMT);
        CHECK_ODBC_OK(SQLGetData(stmt, 1, SQL_C_CHAR, catalog,
                                sizeof(catalog), &indicator),
                      stmt, SQL_HANDLE_STMT);
        CHECK_ODBC_OK(SQLGetData(stmt, 6, SQL_C_CHAR, typeName,
                                sizeof(typeName), &indicator),
                      stmt, SQL_HANDLE_STMT);
        CHECK_ODBC_OK(SQLGetData(stmt, 11, SQL_C_SSHORT, &nullable, 0, &indicator),
                      stmt, SQL_HANDLE_STMT);
        CHECK_ODBC_OK(SQLGetData(stmt, 18, SQL_C_CHAR, isNullable,
                                sizeof(isNullable), &indicator),
                      stmt, SQL_HANDLE_STMT);
        EXPECT_STREQ(columnName, column.Name);
        EXPECT_STREQ(catalog, "local");
        EXPECT_STREQ(typeName, column.TypeName);
        EXPECT_EQ(nullable, column.Nullable);
        EXPECT_STREQ(isNullable, column.IsNullable);
    }
    EXPECT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLPrimaryKeys(stmt, nullptr, 0, nullptr, 0,
                                (SQLCHAR*)table, SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    char columnName[64] = {};
    ASSERT_EQ(SQLGetData(stmt, 4, SQL_C_CHAR, columnName, sizeof(columnName), &indicator), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLGetData(stmt, 1, SQL_C_CHAR, catalog, sizeof(catalog), &indicator),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_STREQ(columnName, "id");
    EXPECT_STREQ(catalog, "local");
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeStmt(stmt, SQL_CLOSE);

    SQLCHAR catalogWithoutSlash[] = "local";
    CHECK_ODBC_OK(SQLTables(stmt, catalogWithoutSlash, SQL_NTS, nullptr, 0,
                           (SQLCHAR*)table, SQL_NTS,
                           (SQLCHAR*)"TABLE", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLGetData(stmt, 1, SQL_C_CHAR, catalog, sizeof(catalog), &indicator),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_STREQ(catalog, "local");
    EXPECT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeStmt(stmt, SQL_CLOSE);

    CHECK_ODBC_OK(SQLPrimaryKeys(stmt, catalogWithoutSlash, SQL_NTS, nullptr, 0,
                                (SQLCHAR*)table, SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLGetData(stmt, 4, SQL_C_CHAR, columnName,
                            sizeof(columnName), &indicator),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_STREQ(columnName, "id");
    EXPECT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeStmt(stmt, SQL_CLOSE);

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
    const char* likePattern = "test_meta_table_%";
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
    SQLCHAR catalog[] = "local";
    SQLCHAR schema[] = "";
    CHECK_ODBC_OK(SQLTables(stmt, catalog, SQL_NTS, schema, 0,
                            (SQLCHAR*)likePattern, SQL_NTS, (SQLCHAR*)"TABLE", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    tableRows = 0;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        ++tableRows;
    }
    ASSERT_EQ(tableRows, 0);
    SQLFreeStmt(stmt, SQL_CLOSE);
    const std::string exactPath = "/local/test_meta_table_1";
    CHECK_ODBC_OK(SQLTables(stmt, catalog, SQL_NTS, schema, 0,
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
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLColumns(stmt, nullptr, 0, nullptr, 0,
                            (SQLCHAR*)".sys/partition_stats", SQL_NTS, nullptr, 0),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(SQLFetch(stmt), SQL_SUCCESS);
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
    SQLCHAR catalog[] = "local";
    SQLCHAR schema[] = "";
    ASSERT_EQ(SQLColumns(stmt, catalog, SQL_NTS, schema, 0,
                        (SQLCHAR*)exactTable.c_str(), SQL_NTS,
                        (SQLCHAR*)"val%", SQL_NTS),
              SQL_SUCCESS);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLColumns(stmt, catalog, SQL_NTS, schema, 0,
                            (SQLCHAR*)exactTable.c_str(), SQL_NTS,
                            (SQLCHAR*)"nonexistent_col%", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_FALSE, 0),
                  stmt, SQL_HANDLE_STMT);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(MetadataApi, SQLTablesFilterByType) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_type_filter", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"CREATE TABLE test_type_filter (id Int32, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLTables(stmt, nullptr, 0, nullptr, 0,
                           (SQLCHAR*)"/local/test_type_filter", SQL_NTS,
                           (SQLCHAR*)"VIEW", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLTables(stmt, nullptr, 0, nullptr, 0,
                           (SQLCHAR*)"/local/test_type_filter", SQL_NTS,
                           (SQLCHAR*)"TABLE", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLTables(stmt, nullptr, 0, nullptr, 0,
                           (SQLCHAR*)"/local/test_type_filter", SQL_NTS,
                           (SQLCHAR*)"", 0),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(MetadataApi, DdlWithComment) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"DROP TABLE IF EXISTS test_ddl_comment", SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"/* ddl */ CREATE TABLE test_ddl_comment (id Int32, PRIMARY KEY (id))", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(MetadataApi, SQLTablesSpecialEnumerationsAndEmptyArguments) {
    TMetadataHandles handles;
    const SQLHSTMT stmt = handles.Stmt;

    SQLCHAR empty[] = "";
    SQLCHAR all[] = "%";
    CHECK_ODBC_OK(SQLTables(
        stmt, all, SQL_NTS,
        empty, 0, empty, 0, empty, 0), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{1, "local"}, {2, nullptr}, {3, nullptr}, {4, nullptr}, {5, nullptr}});
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLTables(
        stmt, empty, 0, all, SQL_NTS,
        empty, 0, empty, 0), stmt, SQL_HANDLE_STMT);
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLTables(
        stmt, empty, 0, empty, 0, empty, 0,
        all, SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    std::set<std::string> tableTypes;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        EXPECT_FALSE(ReadText(stmt, 1));
        EXPECT_FALSE(ReadText(stmt, 2));
        EXPECT_FALSE(ReadText(stmt, 3));
        const auto tableType = ReadText(stmt, 4);
        ASSERT_TRUE(tableType);
        tableTypes.insert(*tableType);
        EXPECT_FALSE(ReadText(stmt, 5));
    }
    EXPECT_TRUE(tableTypes.contains("TABLE"));
    EXPECT_TRUE(tableTypes.contains("VIEW"));
    EXPECT_FALSE(tableTypes.contains("RESOURCE_POOL"));
    EXPECT_FALSE(tableTypes.contains("EXTERNAL_DATA_SOURCE"));
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    CHECK_ODBC_OK(SQLTables(
        stmt, empty, 0, nullptr, 0, nullptr, 0, nullptr, 0),
        stmt, SQL_HANDLE_STMT);
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLTables(
        stmt, nullptr, 0, nullptr, 0, empty, 0, nullptr, 0),
        stmt, SQL_HANDLE_STMT);
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLSetStmtAttr(
        stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_TRUE, 0),
        stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLTables(
        stmt, all, SQL_NTS, empty, 0, empty, 0, empty, 0), stmt, SQL_HANDLE_STMT);
    FinishResult(stmt);
    CHECK_ODBC_OK(SQLTables(
        stmt, empty, 0, empty, 0, empty, 0, all, SQL_NTS), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{4, AnyValue}});
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
}

TEST(MetadataApi, SQLColumnsFieldsAndCompositePrimaryKey) {
    TMetadataHandles handles;
    const SQLHSTMT stmt = handles.Stmt;

    ExecuteAndClose(stmt, "DROP TABLE IF EXISTS test_column_metadata_fields");
    ExecuteAndClose(stmt,
        "CREATE TABLE test_column_metadata_fields ("
        "pk_a Int32, pk_b Int64 NOT NULL, text_value Utf8, payload String, created Timestamp, "
        "PRIMARY KEY (pk_b, pk_a))");
    ExecuteAndClose(stmt,
        "INSERT INTO test_column_metadata_fields (pk_a, pk_b) VALUES (NULL, 1)");
    CHECK_ODBC_OK(SQLExecDirect(stmt,
        (SQLCHAR*)"SELECT pk_a FROM test_column_metadata_fields WHERE pk_b = 1", SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{1, nullptr}});
    FinishResult(stmt);

    struct TExpectedColumn {
        const char* Name;
        SQLSMALLINT DataType;
        SQLSMALLINT Nullable;
        SQLSMALLINT SqlDataType;
        TExpectedValue DateTimeSub;
        TExpectedValue CharOctetLength;
    };
    const std::array expectedColumns{
        TExpectedColumn{"pk_a", SQL_INTEGER, SQL_NULLABLE, SQL_INTEGER, nullptr, nullptr},
        TExpectedColumn{"pk_b", SQL_BIGINT, SQL_NO_NULLS, SQL_BIGINT, nullptr, nullptr},
        TExpectedColumn{"text_value", SQL_VARCHAR, SQL_NULLABLE, SQL_VARCHAR, nullptr, 0},
        TExpectedColumn{"payload", SQL_VARBINARY, SQL_NULLABLE, SQL_VARBINARY, nullptr, 0},
        TExpectedColumn{"created", SQL_TYPE_TIMESTAMP, SQL_NULLABLE, SQL_DATETIME,
                        SQL_CODE_TIMESTAMP, nullptr},
    };

    SQLCHAR tableName[] = "test_column_metadata_fields";
    CHECK_ODBC_OK(SQLColumns(
        stmt, nullptr, 0, nullptr, 0, tableName, SQL_NTS, nullptr, 0),
        stmt, SQL_HANDLE_STMT);
    for (size_t i = 0; i < expectedColumns.size(); ++i) {
        const auto& expected = expectedColumns[i];
        ExpectRow(stmt, {
            {1, "local"}, {2, nullptr}, {3, "test_column_metadata_fields"}, {4, expected.Name},
            {5, expected.DataType}, {8, AnyValue}, {11, expected.Nullable},
            {14, expected.SqlDataType}, {15, expected.DateTimeSub},
            {16, expected.CharOctetLength}, {17, static_cast<SQLINTEGER>(i + 1)},
            {18, expected.Nullable == SQL_NO_NULLS ? "NO" : "YES"},
        });
    }
    FinishResult(stmt);

    SQLCHAR catalog[] = "local";
    SQLCHAR catalogPattern[] = "loca%";
    CHECK_ODBC_OK(SQLColumns(
        stmt, catalog, SQL_NTS, nullptr, 0, tableName, SQL_NTS,
        (SQLCHAR*)"pk_a", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{4, "pk_a"}});
    FinishResult(stmt);
    CHECK_ODBC_OK(SQLColumns(
        stmt, catalogPattern, SQL_NTS, nullptr, 0, tableName, SQL_NTS,
        (SQLCHAR*)"pk_a", SQL_NTS), stmt, SQL_HANDLE_STMT);
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLPrimaryKeys(
        stmt, nullptr, 0, nullptr, 0, tableName, SQL_NTS), stmt, SQL_HANDLE_STMT);
    const std::array<const char*, 2> expectedKeys{"pk_b", "pk_a"};
    for (size_t i = 0; i < expectedKeys.size(); ++i) {
        ExpectRow(stmt, {
            {1, "local"}, {2, nullptr}, {3, "test_column_metadata_fields"},
            {4, expectedKeys[i]}, {5, static_cast<SQLSMALLINT>(i + 1)}, {6, nullptr},
        });
    }
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLSpecialColumns(
        stmt, SQL_BEST_ROWID, nullptr, 0, nullptr, 0, tableName, SQL_NTS,
        SQL_SCOPE_SESSION, SQL_NULLABLE), stmt, SQL_HANDLE_STMT);
    for (const auto* key : expectedKeys) {
        ExpectRow(stmt, {{2, key}});
    }
    FinishResult(stmt);
    CHECK_ODBC_OK(SQLSpecialColumns(
        stmt, SQL_BEST_ROWID, nullptr, 0, nullptr, 0, tableName, SQL_NTS,
        SQL_SCOPE_SESSION, SQL_NO_NULLS), stmt, SQL_HANDLE_STMT);
    FinishResult(stmt);
}

TEST(MetadataApi, SQLGetTypeInfoDecimalParameters) {
    TMetadataHandles handles;
    const SQLHSTMT stmt = handles.Stmt;

    for (const auto type : {SQL_DECIMAL, SQL_NUMERIC}) {
        CHECK_ODBC_OK(SQLGetTypeInfo(stmt, type), stmt, SQL_HANDLE_STMT);
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        const auto name = ReadText(stmt, 1);
        ASSERT_TRUE(name);
        EXPECT_EQ(*name, "Decimal");
        EXPECT_EQ(ReadText(stmt, 3), "35");
        EXPECT_EQ(ReadText(stmt, 6), "precision,scale");
        EXPECT_EQ(ReadText(stmt, 14), "0");
        EXPECT_EQ(ReadText(stmt, 15), "35");
        FinishResult(stmt);

        ExecuteAndClose(stmt, "DROP TABLE IF EXISTS test_decimal_type_info");
        const std::string create = "CREATE TABLE test_decimal_type_info ("
            "id Int32 NOT NULL, value " + *name + "(18, 5), PRIMARY KEY (id))";
        ExecuteAndClose(stmt, create.c_str());
        CHECK_ODBC_OK(SQLColumns(stmt, nullptr, 0, nullptr, 0,
            (SQLCHAR*)"test_decimal_type_info", SQL_NTS, (SQLCHAR*)"value", SQL_NTS),
            stmt, SQL_HANDLE_STMT);
        ExpectRow(stmt, {{5, SQL_DECIMAL}, {7, 18}, {9, 5}});
        FinishResult(stmt);
        ExecuteAndClose(stmt, "DROP TABLE test_decimal_type_info");
    }
}

TEST(MetadataApi, SQLGetTypeInfoTemporalLiterals) {
    TMetadataHandles handles;
    const SQLHSTMT stmt = handles.Stmt;
    struct TLiteralCase {
        SQLSMALLINT Type;
        const char* Value;
        SQL_TIMESTAMP_STRUCT Expected;
    };
    const std::array cases{
        TLiteralCase{SQL_TYPE_DATE, "2024-01-02", {2024, 1, 2, 0, 0, 0, 0}},
        TLiteralCase{SQL_TYPE_TIME, "10:11:12", {1970, 1, 1, 10, 11, 12, 0}},
        TLiteralCase{SQL_TYPE_TIMESTAMP, "2024-01-02 13:14:15.123456",
                     {2024, 1, 2, 13, 14, 15, 123456000}},
    };
    for (const auto& item : cases) {
        SCOPED_TRACE(item.Type);
        CHECK_ODBC_OK(SQLGetTypeInfo(stmt, item.Type), stmt, SQL_HANDLE_STMT);
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        const auto name = ReadText(stmt, 1);
        const auto prefix = ReadText(stmt, 4);
        const auto suffix = ReadText(stmt, 5);
        ASSERT_TRUE(name);
        ASSERT_TRUE(prefix);
        ASSERT_TRUE(suffix);
        FinishResult(stmt);

        ExecuteAndClose(stmt, "DROP TABLE IF EXISTS test_temporal_type_info");
        const std::string create = "CREATE TABLE test_temporal_type_info ("
            "id Int32 NOT NULL, value " + *name + ", PRIMARY KEY (id))";
        ExecuteAndClose(stmt, create.c_str());
        const std::string insert = "--!ansi_lexer\nPRAGMA UnicodeLiterals;\n"
            "INSERT INTO test_temporal_type_info (id, value) VALUES (1, "
            + *prefix + item.Value + *suffix + ")";
        ExecuteAndClose(stmt, insert.c_str());
        CHECK_ODBC_OK(SQLExecDirect(stmt,
            (SQLCHAR*)"SELECT value FROM test_temporal_type_info WHERE id = 1", SQL_NTS),
            stmt, SQL_HANDLE_STMT);
        ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
        SQL_TIMESTAMP_STRUCT actual{};
        SQLLEN indicator = 0;
        CHECK_ODBC_OK(SQLGetData(stmt, 1, SQL_C_TYPE_TIMESTAMP, &actual, sizeof(actual), &indicator),
            stmt, SQL_HANDLE_STMT);
        EXPECT_EQ(indicator, sizeof(actual));
        EXPECT_EQ(actual.year, item.Expected.year);
        EXPECT_EQ(actual.month, item.Expected.month);
        EXPECT_EQ(actual.day, item.Expected.day);
        EXPECT_EQ(actual.hour, item.Expected.hour);
        EXPECT_EQ(actual.minute, item.Expected.minute);
        EXPECT_EQ(actual.second, item.Expected.second);
        EXPECT_EQ(actual.fraction, item.Expected.fraction);
        FinishResult(stmt);
        ExecuteAndClose(stmt, "DROP TABLE test_temporal_type_info");
    }
}

TEST(MetadataApi, MetadataUsesCurrentCatalogForSameNamedTables) {
    TMetadataHandles handles;
    const SQLHDBC dbc = handles.Dbc;
    const SQLHSTMT stmt = handles.Stmt;

    ExecuteAndClose(stmt, "DROP TABLE IF EXISTS `/local/cat_a/catalog_metadata_same`");
    ExecuteAndClose(stmt, "DROP TABLE IF EXISTS `/local/cat_b/catalog_metadata_same`");
    ExecuteAndClose(stmt,
        "CREATE TABLE `/local/cat_a/catalog_metadata_same` ("
        "id Int32, a_value Utf8, PRIMARY KEY (id))");
    ExecuteAndClose(stmt,
        "CREATE TABLE `/local/cat_b/catalog_metadata_same` ("
        "tenant_id Int32, id Int32, b_value Int64, PRIMARY KEY (tenant_id, id))");

    SQLCHAR relativeTable[] = "catalog_metadata_same";
    SQLCHAR catalogA[] = "local/cat_a";
    CHECK_ODBC_OK(SQLSetConnectAttr(
        dbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)"/local/cat_a", SQL_NTS),
        dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLTables(
        stmt, catalogA, SQL_NTS, nullptr, 0, relativeTable, SQL_NTS,
        (SQLCHAR*)"TABLE", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{1, "local/cat_a"}, {3, "catalog_metadata_same"}});
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLColumns(
        stmt, nullptr, 0, nullptr, 0, relativeTable, SQL_NTS, nullptr, 0),
        stmt, SQL_HANDLE_STMT);
    std::vector<std::string> columns;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        const auto name = ReadText(stmt, 4);
        ASSERT_TRUE(name);
        columns.push_back(*name);
    }
    EXPECT_EQ(columns, (std::vector<std::string>{"id", "a_value"}));
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    CHECK_ODBC_OK(SQLSetConnectAttr(
        dbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)"/local/cat_b", SQL_NTS),
        dbc, SQL_HANDLE_DBC);
    CHECK_ODBC_OK(SQLPrimaryKeys(
        stmt, nullptr, 0, nullptr, 0, relativeTable, SQL_NTS),
        stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{1, "local/cat_b"}, {4, "tenant_id"}, {5, 1}});
    ExpectRow(stmt, {{4, "id"}, {5, 2}});
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLSetConnectAttr(
        dbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER)"/local", SQL_NTS),
        dbc, SQL_HANDLE_DBC);
    SQLCHAR nestedTable[] = "cat_a/catalog_metadata_same";
    CHECK_ODBC_OK(SQLTables(
        stmt, nullptr, 0, nullptr, 0, nestedTable, SQL_NTS,
        (SQLCHAR*)"TABLE", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{3, "cat_a/catalog_metadata_same"}});
    FinishResult(stmt);

    SQLCHAR absoluteTable[] = "/local/cat_b/catalog_metadata_same";
    CHECK_ODBC_OK(SQLColumns(
        stmt, nullptr, 0, nullptr, 0, absoluteTable, SQL_NTS,
        (SQLCHAR*)"b_value", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{3, "cat_b/catalog_metadata_same"}, {4, "b_value"}});
    FinishResult(stmt);

    SQLCHAR allTables[] = "%";
    CHECK_ODBC_OK(SQLTables(
        stmt, nullptr, 0, nullptr, 0, allTables, SQL_NTS,
        (SQLCHAR*)"TABLE", SQL_NTS), stmt, SQL_HANDLE_STMT);
    std::set<std::string> names;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        const auto name = ReadText(stmt, 3);
        ASSERT_TRUE(name);
        names.insert(*name);
    }
    EXPECT_TRUE(names.contains("cat_a/catalog_metadata_same"));
    EXPECT_TRUE(names.contains("cat_b/catalog_metadata_same"));
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    SQLCHAR nestedPattern[] = "cat_a/catalog_metadata_%";
    CHECK_ODBC_OK(SQLTables(
        stmt, nullptr, 0, nullptr, 0, nestedPattern, SQL_NTS,
        (SQLCHAR*)"TABLE", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{3, "cat_a/catalog_metadata_same"}});
    FinishResult(stmt);
}

TEST(MetadataApi, SQLStatisticsReportsIndexesAndHonorsOptions) {
    TMetadataHandles handles;
    const SQLHSTMT stmt = handles.Stmt;

    ExecuteAndClose(stmt, "DROP TABLE IF EXISTS test_statistics_metadata");
    ExecuteAndClose(stmt,
        "CREATE TABLE test_statistics_metadata ("
        "id Int32, city Utf8, created Timestamp, "
        "INDEX idx_city_created GLOBAL SYNC ON (city, created), "
        "PRIMARY KEY (id))");
    ExecuteAndClose(stmt,
        "UPSERT INTO test_statistics_metadata (id, city) VALUES "
        "(1, 'A'), (2, 'B')");

    SQLCHAR tableName[] = "test_statistics_metadata";
    CHECK_ODBC_OK(SQLStatistics(
        stmt, nullptr, 0, nullptr, 0, tableName, SQL_NTS,
        SQL_INDEX_ALL, SQL_QUICK), stmt, SQL_HANDLE_STMT);

    ExpectRow(stmt, {
        {3, "test_statistics_metadata"}, {4, nullptr}, {6, nullptr}, {7, SQL_TABLE_STAT},
        {8, nullptr}, {9, nullptr}, {10, nullptr}, {11, nullptr}, {12, nullptr}, {13, nullptr},
    });
    const std::array<const char*, 2> regularIndexColumns{"city", "created"};
    for (size_t i = 0; i < regularIndexColumns.size(); ++i) {
        ExpectRow(stmt, {
            {4, SQL_TRUE}, {6, "idx_city_created"}, {7, SQL_INDEX_OTHER},
            {8, static_cast<SQLSMALLINT>(i + 1)}, {9, regularIndexColumns[i]},
            {11, nullptr}, {12, nullptr},
        });
    }
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLStatistics(
        stmt, nullptr, 0, nullptr, 0, tableName, SQL_NTS,
        SQL_INDEX_UNIQUE, SQL_ENSURE), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{7, SQL_TABLE_STAT}, {11, AnyValue}});
    FinishResult(stmt);
}

TEST(MetadataApi, SQLStatisticsUniqueIndexWhenSupported) {
    TMetadataHandles handles;
    const SQLHSTMT stmt = handles.Stmt;

    ExecuteAndClose(stmt, "DROP TABLE IF EXISTS test_statistics_unique_metadata");
    const char* createTable =
        "CREATE TABLE test_statistics_unique_metadata ("
        "id Int32, email Utf8 NOT NULL, city Utf8, "
        "INDEX idx_city GLOBAL SYNC ON (city), "
        "INDEX idx_email GLOBAL UNIQUE SYNC ON (email), "
        "PRIMARY KEY (id))";
    const SQLRETURN rc = SQLExecDirect(
        stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>(createTable)), SQL_NTS);
    if (rc == SQL_ERROR) {
        const std::string error = GetOdbcError(stmt, SQL_HANDLE_STMT);
        if (error.find("Unique constraint feature is disabled") != std::string::npos) {
            GTEST_SKIP() << error;
        }
    }
    CHECK_ODBC_OK(rc, stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);

    SQLCHAR tableName[] = "test_statistics_unique_metadata";
    CHECK_ODBC_OK(SQLStatistics(
        stmt, nullptr, 0, nullptr, 0, tableName, SQL_NTS,
        SQL_INDEX_ALL, SQL_QUICK), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{7, SQL_TABLE_STAT}});
    ExpectRow(stmt, {{4, SQL_FALSE}, {6, "idx_email"}, {8, 1}, {9, "email"}});
    ExpectRow(stmt, {{4, SQL_TRUE}, {6, "idx_city"}, {8, 1}, {9, "city"}});
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLStatistics(
        stmt, nullptr, 0, nullptr, 0, tableName, SQL_NTS,
        SQL_INDEX_UNIQUE, SQL_QUICK), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{7, SQL_TABLE_STAT}});
    ExpectRow(stmt, {
        {4, SQL_FALSE}, {6, "idx_email"}, {7, SQL_INDEX_OTHER}, {8, 1}, {9, "email"},
    });
    FinishResult(stmt);
}

TEST(MetadataApi, RequiredTableArgumentsAndStatisticsOptionsAreValidated) {
    TMetadataHandles handles;
    const SQLHSTMT stmt = handles.Stmt;

    ExpectError(SQLPrimaryKeys(stmt, nullptr, 0, nullptr, 0, nullptr, 0), stmt, "HY009");
    ExpectError(SQLStatistics(
        stmt, nullptr, 0, nullptr, 0, nullptr, 0, SQL_INDEX_ALL, SQL_QUICK), stmt, "HY009");

    SQLCHAR tableName[] = "not_used";
    ExpectError(SQLStatistics(
        stmt, nullptr, 0, nullptr, 0, tableName, SQL_NTS, 99, SQL_QUICK), stmt, "HY100");
    ExpectError(SQLStatistics(
        stmt, nullptr, 0, nullptr, 0, tableName, SQL_NTS, SQL_INDEX_ALL, 99), stmt, "HY101");

    CHECK_ODBC_OK(SQLSetStmtAttr(
        stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_TRUE, 0),
        stmt, SQL_HANDLE_STMT);
    ExpectError(SQLTables(
        stmt, nullptr, 0, nullptr, 0, tableName, SQL_NTS, nullptr, 0), stmt, "HY009");
}

TEST(MetadataApi, SQLSpecialColumnsDistinguishesNullAndEmptyArguments) {
    TMetadataHandles handles;
    const SQLHSTMT stmt = handles.Stmt;

    ExecuteAndClose(stmt, "DROP TABLE IF EXISTS test_special_columns_metadata");
    ExecuteAndClose(stmt,
        "CREATE TABLE test_special_columns_metadata (id Int32 NOT NULL, PRIMARY KEY (id))");

    SQLCHAR tableName[] = "test_special_columns_metadata";
    SQLCHAR catalog[] = "local";
    SQLCHAR empty[] = "";
    ExpectError(SQLSpecialColumns(
        stmt, SQL_BEST_ROWID, nullptr, 0, nullptr, 0, nullptr, 0,
        SQL_SCOPE_SESSION, SQL_NULLABLE), stmt, "HY009");

    CHECK_ODBC_OK(SQLSpecialColumns(
        stmt, SQL_BEST_ROWID, empty, 0, nullptr, 0, tableName, SQL_NTS,
        SQL_SCOPE_SESSION, SQL_NULLABLE), stmt, SQL_HANDLE_STMT);
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLSetStmtAttr(
        stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)(uintptr_t)SQL_TRUE, 0),
        stmt, SQL_HANDLE_STMT);
    ExpectError(SQLSpecialColumns(
        stmt, SQL_BEST_ROWID, nullptr, 0, empty, 0, tableName, SQL_NTS,
        SQL_SCOPE_SESSION, SQL_NULLABLE), stmt, "HY009");
    ExpectError(SQLSpecialColumns(
        stmt, SQL_BEST_ROWID, catalog, SQL_NTS, nullptr, 0, tableName, SQL_NTS,
        SQL_SCOPE_SESSION, SQL_NULLABLE), stmt, "HY009");

    CHECK_ODBC_OK(SQLSpecialColumns(
        stmt, SQL_BEST_ROWID, catalog, SQL_NTS, empty, 0, tableName, SQL_NTS,
        SQL_SCOPE_SESSION, SQL_NULLABLE), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{2, "id"}});
    FinishResult(stmt);

    CHECK_ODBC_OK(SQLSpecialColumns(
        stmt, SQL_BEST_ROWID, catalog, SQL_NTS, empty, 0, tableName, SQL_NTS,
        SQL_SCOPE_SESSION, SQL_NO_NULLS), stmt, SQL_HANDLE_STMT);
    ExpectRow(stmt, {{2, "id"}});
    FinishResult(stmt);
    ExpectError(SQLSpecialColumns(
        stmt, SQL_BEST_ROWID, catalog, SQL_NTS, empty, 0, tableName, SQL_NTS,
        SQL_SCOPE_SESSION, SQL_NULLABLE_UNKNOWN), stmt, "HY099");
}
