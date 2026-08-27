#include "test_utils.h"

#include <array>
#include <cstring>

#ifndef SQL_ODBC_INTERFACE_CONFORMANCE
#define SQL_ODBC_INTERFACE_CONFORMANCE 169
#endif

TEST(CoreApi, SQLGetTypeInfoAll) {
    struct TExpectedType {
        SQLSMALLINT DataType;
        const char* TypeName;
    };
    constexpr std::array<TExpectedType, 7> expected{{
        {SQL_BIGINT, "Int64"},
        {SQL_INTEGER, "Int32"},
        {SQL_SMALLINT, "Int16"},
        {SQL_DOUBLE, "Double"},
        {SQL_REAL, "Float"},
        {SQL_VARCHAR, "Utf8"},
        {SQL_CHAR, "Utf8"},
    }};

    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLGetTypeInfo(stmt, SQL_ALL_TYPES), stmt, SQL_HANDLE_STMT);

    char typeName[64] = {};
    SQLSMALLINT dataType = 0;
    SQLLEN typeNameIndicator = 0;
    SQLLEN dataTypeIndicator = 0;
    CHECK_ODBC_OK(SQLBindCol(stmt, 1, SQL_C_CHAR, typeName, sizeof(typeName),
                            &typeNameIndicator),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindCol(stmt, 2, SQL_C_SSHORT, &dataType, 0, &dataTypeIndicator),
                  stmt, SQL_HANDLE_STMT);

    std::array<bool, expected.size()> seen{};
    size_t rowCount = 0;
    SQLRETURN fetchResult;
    while ((fetchResult = SQLFetch(stmt)) == SQL_SUCCESS) {
        ++rowCount;
        bool matched = false;
        for (size_t index = 0; index < expected.size(); ++index) {
            if (dataType == expected[index].DataType
                && std::strcmp(typeName, expected[index].TypeName) == 0) {
                EXPECT_FALSE(seen[index]) << "duplicate type row " << typeName;
                seen[index] = true;
                matched = true;
                break;
            }
        }
        EXPECT_TRUE(matched) << "unexpected advertised type " << typeName
                             << " (" << dataType << ")";
    }
    EXPECT_EQ(fetchResult, SQL_NO_DATA);
    EXPECT_EQ(rowCount, expected.size());
    for (size_t index = 0; index < expected.size(); ++index) {
        EXPECT_TRUE(seen[index]) << "missing " << expected[index].TypeName
                                 << " for SQL type " << expected[index].DataType;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLGetTypeInfoSchema) {
    struct TExpectedColumn {
        const char* Name;
        SQLSMALLINT Type;
        SQLSMALLINT Nullable;
    };
    constexpr std::array<TExpectedColumn, 19> expected{{
        {"TYPE_NAME", SQL_VARCHAR, SQL_NO_NULLS},
        {"DATA_TYPE", SQL_SMALLINT, SQL_NO_NULLS},
        {"COLUMN_SIZE", SQL_INTEGER, SQL_NULLABLE},
        {"LITERAL_PREFIX", SQL_VARCHAR, SQL_NULLABLE},
        {"LITERAL_SUFFIX", SQL_VARCHAR, SQL_NULLABLE},
        {"CREATE_PARAMS", SQL_VARCHAR, SQL_NULLABLE},
        {"NULLABLE", SQL_SMALLINT, SQL_NO_NULLS},
        {"CASE_SENSITIVE", SQL_SMALLINT, SQL_NO_NULLS},
        {"SEARCHABLE", SQL_SMALLINT, SQL_NO_NULLS},
        {"UNSIGNED_ATTRIBUTE", SQL_SMALLINT, SQL_NULLABLE},
        {"FIXED_PREC_SCALE", SQL_SMALLINT, SQL_NO_NULLS},
        {"AUTO_UNIQUE_VALUE", SQL_SMALLINT, SQL_NULLABLE},
        {"LOCAL_TYPE_NAME", SQL_VARCHAR, SQL_NULLABLE},
        {"MINIMUM_SCALE", SQL_SMALLINT, SQL_NULLABLE},
        {"MAXIMUM_SCALE", SQL_SMALLINT, SQL_NULLABLE},
        {"SQL_DATA_TYPE", SQL_SMALLINT, SQL_NO_NULLS},
        {"SQL_DATETIME_SUB", SQL_SMALLINT, SQL_NULLABLE},
        {"NUM_PREC_RADIX", SQL_INTEGER, SQL_NULLABLE},
        {"INTERVAL_PRECISION", SQL_SMALLINT, SQL_NULLABLE},
    }};

    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLGetTypeInfo(stmt, SQL_ALL_TYPES), stmt, SQL_HANDLE_STMT);

    SQLSMALLINT columnCount = 0;
    CHECK_ODBC_OK(SQLNumResultCols(stmt, &columnCount), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(columnCount, static_cast<SQLSMALLINT>(expected.size()));
    for (size_t index = 0; index < expected.size(); ++index) {
        const SQLUSMALLINT column = static_cast<SQLUSMALLINT>(index + 1);
        SCOPED_TRACE(column);
        char name[64] = {};
        SQLSMALLINT nameLength = 0;
        SQLSMALLINT type = 0;
        SQLULEN size = 0;
        SQLSMALLINT scale = 0;
        SQLSMALLINT nullable = 0;
        CHECK_ODBC_OK(SQLDescribeCol(
                          stmt, column, reinterpret_cast<SQLCHAR*>(name), sizeof(name),
                          &nameLength, &type, &size, &scale, &nullable),
                      stmt, SQL_HANDLE_STMT);
        EXPECT_STREQ(name, expected[index].Name);
        EXPECT_EQ(nameLength, static_cast<SQLSMALLINT>(std::strlen(expected[index].Name)));
        EXPECT_EQ(type, expected[index].Type);
        EXPECT_EQ(nullable, expected[index].Nullable);
    }

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

TEST(CoreApi, SQLGetTypeInfoTimestampAlias) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLGetTypeInfo(stmt, SQL_TIMESTAMP), stmt, SQL_HANDLE_STMT);
    SQLINTEGER columnSize = 0;
    SQLLEN indicator = 0;
    SQLBindCol(stmt, 3, SQL_C_LONG, &columnSize, 0, &indicator);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    EXPECT_EQ(columnSize, 26);
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
    EXPECT_EQ(outLen, 8);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLNativeSqlTranslatesOdbcSyntax) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    std::string input =
        "SELECT {fn CONVERT(?, SQL_SMALLINT)} AS \"value{fn ABS(1)}\", "
        "'{fn ABS(2)}' /* {fn ABS(3)} */";
    const std::string expected =
        "SELECT CAST(? AS Int16) AS \"value{fn ABS(1)}\", "
        "'{fn ABS(2)}' /* {fn ABS(3)} */";
    char out[256] = {};
    SQLINTEGER outLen = 0;
    CHECK_ODBC_OK(SQLNativeSql(
        dbc, reinterpret_cast<SQLCHAR*>(input.data()), SQL_NTS,
        reinterpret_cast<SQLCHAR*>(out), sizeof(out), &outLen),
        dbc, SQL_HANDLE_DBC);
    EXPECT_STREQ(out, expected.c_str());
    EXPECT_EQ(outLen, static_cast<SQLINTEGER>(expected.size()));
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLNativeSqlPreservesLargeExplicitLength) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);
    std::string input(40000, 'x');
    std::string output(input.size() + 1, '\0');
    SQLINTEGER outLen = -1;
    CHECK_ODBC_OK(SQLNativeSql(
        dbc, reinterpret_cast<SQLCHAR*>(input.data()), static_cast<SQLINTEGER>(input.size()),
        reinterpret_cast<SQLCHAR*>(output.data()), static_cast<SQLINTEGER>(output.size()), &outLen),
        dbc, SQL_HANDLE_DBC);
    EXPECT_EQ(outLen, static_cast<SQLINTEGER>(input.size()));
    output.resize(static_cast<size_t>(outLen));
    EXPECT_EQ(output, input);
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
    SQLCHAR columnName[32] = {};
    SQLSMALLINT nameLength = 0;
    SQLSMALLINT dataType = 0;
    SQLULEN columnSize = 0;
    SQLSMALLINT decimalDigits = 0;
    SQLSMALLINT nullable = 0;
    CHECK_ODBC_OK(SQLDescribeCol(stmt, 4, columnName, sizeof(columnName), &nameLength,
                                &dataType, &columnSize, &decimalDigits, &nullable),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_STREQ(reinterpret_cast<char*>(columnName), "NON_UNIQUE");
    EXPECT_EQ(dataType, SQL_SMALLINT);
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
    char userName[8] = {'x'};
    CHECK_ODBC_OK(SQLGetInfo(dbc, SQL_USER_NAME, userName, sizeof(userName), &outLen),
                  dbc, SQL_HANDLE_DBC);
    EXPECT_STREQ(userName, "");
    EXPECT_EQ(outLen, 0);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLGetInfoScalarWidths) {
    struct TU16Info {
        SQLUSMALLINT InfoType;
        SQLUSMALLINT Expected;
    };
    struct TU32Info {
        SQLUSMALLINT InfoType;
        SQLUINTEGER Expected;
    };
    constexpr std::array<TU16Info, 11> u16Info{{
        {SQL_MAX_DRIVER_CONNECTIONS, 0},
        {SQL_MAX_CONCURRENT_ACTIVITIES, 0},
        {SQL_MAX_COLUMNS_IN_GROUP_BY, 0},
        {SQL_MAX_COLUMNS_IN_ORDER_BY, 0},
        {SQL_MAX_COLUMNS_IN_INDEX, 20},
        {SQL_MAX_COLUMNS_IN_SELECT, 0},
        {SQL_MAX_COLUMNS_IN_TABLE, 200},
        {SQL_MAX_TABLES_IN_SELECT, 0},
        {SQL_CATALOG_LOCATION, SQL_CL_START},
        {SQL_GROUP_BY, SQL_GB_GROUP_BY_CONTAINS_SELECT},
        {SQL_NON_NULLABLE_COLUMNS, SQL_NNC_NON_NULL},
    }};
    constexpr std::array<TU32Info, 4> u32Info{{
        {SQL_ALTER_TABLE, 0},
        {SQL_CATALOG_USAGE, 0},
        {SQL_DYNAMIC_CURSOR_ATTRIBUTES1, 0},
        {SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2, SQL_CA2_READ_ONLY_CONCURRENCY},
    }};
    constexpr SQLUSMALLINT u16Canary = 0xA55A;
    constexpr SQLUINTEGER u32Canary = 0xA55AA55A;

    SQLHENV env;
    SQLHDBC dbc;
    AllocEnvAndConnect(&env, &dbc);

    for (const auto& info : u16Info) {
        SCOPED_TRACE(info.InfoType);
        std::array<SQLUSMALLINT, 2> guarded{0xFFFF, u16Canary};
        SQLSMALLINT outLength = -1;
        CHECK_ODBC_OK(SQLGetInfo(dbc, info.InfoType, guarded.data(), sizeof(guarded[0]), &outLength),
                      dbc, SQL_HANDLE_DBC);
        EXPECT_EQ(guarded[0], info.Expected);
        EXPECT_EQ(guarded[1], u16Canary);
        EXPECT_EQ(outLength, static_cast<SQLSMALLINT>(sizeof(SQLUSMALLINT)));
    }

    for (const auto& info : u32Info) {
        SCOPED_TRACE(info.InfoType);
        std::array<SQLUINTEGER, 2> guarded{0xFFFFFFFF, u32Canary};
        SQLSMALLINT outLength = -1;
        CHECK_ODBC_OK(SQLGetInfo(dbc, info.InfoType, guarded.data(), sizeof(guarded[0]), &outLength),
                      dbc, SQL_HANDLE_DBC);
        EXPECT_EQ(guarded[0], info.Expected);
        EXPECT_EQ(guarded[1], u32Canary);
        EXPECT_EQ(outLength, static_cast<SQLSMALLINT>(sizeof(SQLUINTEGER)));
    }

    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(CoreApi, SQLSetGetStmtAttrCursorCapabilities) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLULEN value = static_cast<SQLULEN>(-1);
    CHECK_ODBC_OK(SQLGetStmtAttr(
                      stmt, SQL_ATTR_CURSOR_SENSITIVITY, &value, sizeof(value), nullptr),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(value, SQL_UNSPECIFIED);
    CHECK_ODBC_OK(SQLGetStmtAttr(
                      stmt, SQL_ATTR_USE_BOOKMARKS, &value, sizeof(value), nullptr),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(value, SQL_UB_OFF);

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_SENSITIVITY,
                                (SQLPOINTER)SQL_INSENSITIVE, 0),
                  stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLGetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE, &value, sizeof(value), nullptr),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(value, SQL_CURSOR_STATIC);
    CHECK_ODBC_OK(SQLGetStmtAttr(
                      stmt, SQL_ATTR_CURSOR_SENSITIVITY, &value, sizeof(value), nullptr),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(value, SQL_INSENSITIVE);

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_SENSITIVITY,
                             (SQLPOINTER)SQL_SENSITIVE, 0),
              SQL_SUCCESS_WITH_INFO);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "01S02"));
    CHECK_ODBC_OK(SQLGetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE, &value, sizeof(value), nullptr),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(value, SQL_CURSOR_STATIC);
    CHECK_ODBC_OK(SQLGetStmtAttr(
                      stmt, SQL_ATTR_CURSOR_SENSITIVITY, &value, sizeof(value), nullptr),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(value, SQL_INSENSITIVE);

    CHECK_ODBC_OK(SQLSetStmtAttr(stmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0),
                  stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLSetStmtAttr(stmt, SQL_ATTR_USE_BOOKMARKS, (SQLPOINTER)SQL_UB_ON, 0),
              SQL_SUCCESS_WITH_INFO);
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(stmt, SQL_HANDLE_STMT), "01S02"));
    CHECK_ODBC_OK(SQLGetStmtAttr(
                      stmt, SQL_ATTR_USE_BOOKMARKS, &value, sizeof(value), nullptr),
                  stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(value, SQL_UB_OFF);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
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

TEST(CoreApi, SQLColumnPrivilegesEmpty) {
    struct TExpectedColumn {
        const char* Name;
        SQLSMALLINT Type;
        SQLSMALLINT Nullable;
    };
    constexpr std::array<TExpectedColumn, 8> expected{{
        {"TABLE_CAT", SQL_VARCHAR, SQL_NULLABLE},
        {"TABLE_SCHEM", SQL_VARCHAR, SQL_NULLABLE},
        {"TABLE_NAME", SQL_VARCHAR, SQL_NO_NULLS},
        {"COLUMN_NAME", SQL_VARCHAR, SQL_NO_NULLS},
        {"GRANTOR", SQL_VARCHAR, SQL_NULLABLE},
        {"GRANTEE", SQL_VARCHAR, SQL_NO_NULLS},
        {"PRIVILEGE", SQL_VARCHAR, SQL_NO_NULLS},
        {"IS_GRANTABLE", SQL_VARCHAR, SQL_NULLABLE},
    }};

    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLUSMALLINT supported = SQL_FALSE;
    CHECK_ODBC_OK(SQLGetFunctions(dbc, SQL_API_SQLCOLUMNPRIVILEGES, &supported),
                  dbc, SQL_HANDLE_DBC);
    EXPECT_EQ(supported, SQL_TRUE);

    CHECK_ODBC_OK(SQLColumnPrivileges(stmt, nullptr, 0, nullptr, 0,
                                      (SQLCHAR*)"does_not_exist", SQL_NTS,
                                      (SQLCHAR*)"%", SQL_NTS),
                  stmt, SQL_HANDLE_STMT);
    SQLSMALLINT columnCount = 0;
    CHECK_ODBC_OK(SQLNumResultCols(stmt, &columnCount), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(columnCount, static_cast<SQLSMALLINT>(expected.size()));
    for (size_t index = 0; index < expected.size(); ++index) {
        char name[32] = {};
        SQLSMALLINT nameLength = 0;
        SQLSMALLINT type = 0;
        SQLULEN size = 0;
        SQLSMALLINT scale = 0;
        SQLSMALLINT nullable = 0;
        CHECK_ODBC_OK(SQLDescribeCol(
                          stmt, static_cast<SQLUSMALLINT>(index + 1),
                          reinterpret_cast<SQLCHAR*>(name), sizeof(name), &nameLength,
                          &type, &size, &scale, &nullable),
                      stmt, SQL_HANDLE_STMT);
        EXPECT_STREQ(name, expected[index].Name);
        EXPECT_EQ(type, expected[index].Type);
        EXPECT_EQ(nullable, expected[index].Nullable);
    }
    EXPECT_EQ(SQLFetch(stmt), SQL_NO_DATA);

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
    SQLPOINTER parameterToken = &atExec;
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 32, 0,
                                   parameterToken, 0, &atExec), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLExecute(stmt), SQL_NEED_DATA);
    SQLPOINTER token = nullptr;
    ASSERT_EQ(SQLParamData(stmt, &token), SQL_NEED_DATA);
    EXPECT_EQ(token, parameterToken);
    const char part1[] = "hel";
    CHECK_ODBC_OK(SQLPutData(stmt, (SQLPOINTER)part1, sizeof(part1) - 1), stmt, SQL_HANDLE_STMT);
    const char part2[] = "lo";
    CHECK_ODBC_OK(SQLPutData(stmt, (SQLPOINTER)part2, sizeof(part2) - 1), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLPutData(stmt, (SQLPOINTER)"", 0), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLParamData(stmt, &token), stmt, SQL_HANDLE_STMT);
    EXPECT_EQ(SQLExecute(stmt), SQL_NEED_DATA);
    CHECK_ODBC_OK(SQLCancel(stmt), stmt, SQL_HANDLE_STMT);
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
    SQLHSTMT first;
    SQLHSTMT second;
    SQLHDESC sharedApd;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &first), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &second), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, dbc, &sharedApd), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLPrepare(first, (SQLCHAR*)"SELECT ?", SQL_NTS), first, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLPrepare(second, (SQLCHAR*)"SELECT ?", SQL_NTS), second, SQL_HANDLE_STMT);
    SQLLEN atExec = SQL_DATA_AT_EXEC;
    SQLINTEGER tokenMarker = 0;
    CHECK_ODBC_OK(SQLBindParameter(second, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 32, 0,
                                   &tokenMarker, 0, &atExec), second, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetStmtAttr(first, SQL_ATTR_APP_PARAM_DESC, sharedApd, 0), first, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLBindParameter(first, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 32, 0,
                                   &tokenMarker, 0, &atExec), first, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLSetStmtAttr(second, SQL_ATTR_APP_PARAM_DESC, sharedApd, 0), second, SQL_HANDLE_STMT);

    SQLPOINTER token = nullptr;
    ASSERT_EQ(SQLExecute(first), SQL_NEED_DATA);
    ASSERT_EQ(SQLParamData(first, &token), SQL_NEED_DATA);
    CHECK_ODBC_OK(SQLPutData(first, (SQLPOINTER)"first", SQL_NTS), first, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLExecute(second), SQL_NEED_DATA);
    ASSERT_EQ(SQLParamData(second, &token), SQL_NEED_DATA);
    ASSERT_EQ(SQLPutData(second, (SQLPOINTER)"", 0), SQL_SUCCESS);
    const SQLRETURN secondRc = SQLParamData(second, &token);
    CHECK_ODBC_OK(secondRc, second, SQL_HANDLE_STMT);
    const SQLRETURN firstRc = SQLParamData(first, &token);
    CHECK_ODBC_OK(firstRc, first, SQL_HANDLE_STMT);

    SQLHSTMT statements[] = {first, second};
    const char* expected[] = {"first", ""};
    for (size_t i = 0; i < 2; ++i) {
        ASSERT_EQ(SQLFetch(statements[i]), SQL_SUCCESS);
        char actual[8] = {};
        CHECK_ODBC_OK(SQLGetData(statements[i], 1, SQL_C_CHAR, actual, sizeof(actual), nullptr),
                      statements[i], SQL_HANDLE_STMT);
        EXPECT_STREQ(actual, expected[i]);
        SQLFreeHandle(SQL_HANDLE_STMT, statements[i]);
    }
    SQLFreeHandle(SQL_HANDLE_DESC, sharedApd);
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
