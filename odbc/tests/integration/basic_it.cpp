#include <gtest/gtest.h>

#include <sql.h>
#include <sqlext.h>

#include <string>


#define CHECK_ODBC_OK(rc, handle, type) \
    ASSERT_TRUE((rc) == SQL_SUCCESS || (rc) == SQL_SUCCESS_WITH_INFO) << GetOdbcError(handle, type)

std::string GetOdbcError(SQLHANDLE handle, SQLSMALLINT type) {
    SQLCHAR sqlState[6], message[256];
    SQLINTEGER nativeError;
    SQLSMALLINT textLength;
    SQLRETURN rc = SQLGetDiagRec(type, handle, 1, sqlState, &nativeError, message, sizeof(message), &textLength);
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
        return std::string((char*)sqlState) + ": " + (char*)message;
    }
    return "Unknown ODBC error";
}

const char* kConnStr = "Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;";

TEST(OdbcBasic, SimpleQuery) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLDriverConnect(dbc, nullptr, (SQLCHAR*)kConnStr, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE), dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    // Simple query
    CHECK_ODBC_OK(SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1 AS one, 'abc' AS str", SQL_NTS), stmt, SQL_HANDLE_STMT);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);

    SQLINTEGER ival = 0;
    char sval[16] = {0};
    SQLLEN ival_ind = 0, sval_ind = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &ival, 0, &ival_ind), SQL_SUCCESS);
    ASSERT_EQ(SQLGetData(stmt, 2, SQL_C_CHAR, sval, sizeof(sval), &sval_ind), SQL_SUCCESS);
    ASSERT_EQ(ival, 1);
    ASSERT_STREQ(sval, "abc");
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcBasic, ParameterizedQuery) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLDriverConnect(dbc, nullptr, (SQLCHAR*)kConnStr, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE), dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLCHAR query[] = R"(
        DECLARE $p1 AS Int32?;
        SELECT $p1 + 10 AS res;
    )";

    // Parameterized query
    CHECK_ODBC_OK(SQLPrepare(stmt, query, SQL_NTS), stmt, SQL_HANDLE_STMT);
    SQLINTEGER param = 5;
    CHECK_ODBC_OK(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &param, 0, nullptr), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecute(stmt), stmt, SQL_HANDLE_STMT);

    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);

    SQLINTEGER res = 0;
    SQLLEN res_ind = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &res, 0, &res_ind), SQL_SUCCESS);
    ASSERT_EQ(res, 15);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(OdbcBasic, ColumnBinding) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env), SQL_SUCCESS);
    ASSERT_EQ(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    CHECK_ODBC_OK(SQLDriverConnect(dbc, nullptr, (SQLCHAR*)kConnStr, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE), dbc, SQL_HANDLE_DBC);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);

    SQLCHAR query_ddl[] = R"(
        DROP TABLE IF EXISTS test_bind;
        CREATE TABLE test_bind (id Int32, name Text, PRIMARY KEY (id));
    )";

    SQLCHAR query[] = R"(
        UPSERT INTO test_bind (id, name) VALUES (1, 'foo'), (2, 'bar');
        SELECT id, name FROM test_bind ORDER BY id;
    )";

    CHECK_ODBC_OK(SQLExecDirect(stmt, query_ddl, SQL_NTS), stmt, SQL_HANDLE_STMT);
    CHECK_ODBC_OK(SQLExecDirect(stmt, query, SQL_NTS), stmt, SQL_HANDLE_STMT);

    SQLINTEGER id = 0;
    char name[16] = {0};
    SQLLEN id_ind = 0, name_ind = 0;
    ASSERT_EQ(SQLBindCol(stmt, 1, SQL_C_LONG, &id, 0, &id_ind), SQL_SUCCESS);
    ASSERT_EQ(SQLBindCol(stmt, 2, SQL_C_CHAR, name, sizeof(name), &name_ind), SQL_SUCCESS);
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(id, 1);
    ASSERT_STREQ(name, "foo");
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    ASSERT_EQ(id, 2);
    ASSERT_STREQ(name, "bar");
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}
