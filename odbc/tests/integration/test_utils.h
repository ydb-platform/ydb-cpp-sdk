#pragma once

#include <gtest/gtest.h>

#include <sql.h>
#include <sqlext.h>

#include <cstring>
#include <string>

inline std::string GetOdbcError(SQLHANDLE handle, SQLSMALLINT type) {
    SQLCHAR sqlState[6] = {0};
    SQLCHAR message[256] = {0};
    SQLINTEGER nativeError = 0;
    SQLSMALLINT textLength = 0;
    SQLRETURN rc = SQLGetDiagRec(type, handle, 1, sqlState, &nativeError, message, sizeof(message), &textLength);
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
        return std::string((char*)sqlState) + ": " + (char*)message;
    }
    return "Unknown ODBC error";
}

#define CHECK_ODBC_OK(rc, handle, type) \
    ASSERT_TRUE((rc) == SQL_SUCCESS || (rc) == SQL_SUCCESS_WITH_INFO) << GetOdbcError(handle, type)

inline const char* kConnStr = "Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;";

inline bool SqlStatePrefix(const std::string& diag, const char* state5) {
    return diag.size() >= 5 && std::strncmp(diag.c_str(), state5, 5) == 0;
}

inline void AllocEnv(SQLHENV* env) {
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, env), SQL_SUCCESS);
    ASSERT_EQ(SQLSetEnvAttr(*env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), SQL_SUCCESS);
}

inline void AllocEnvAndConnect(SQLHENV* env, SQLHDBC* dbc) {
    AllocEnv(env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, *env, dbc), SQL_SUCCESS);
    SQLRETURN rc = SQLDriverConnect(
        *dbc, nullptr, (SQLCHAR*)kConnStr, SQL_NTS, nullptr, 0, nullptr, SQL_DRIVER_COMPLETE);
    CHECK_ODBC_OK(rc, *dbc, SQL_HANDLE_DBC);
}
