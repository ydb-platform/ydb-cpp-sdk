#pragma once

#include <gtest/gtest.h>

#include <sql.h>
#include <sqlext.h>

#include <string>

#define CHECK_ODBC_OK(rc, handle, type) \
    ASSERT_TRUE((rc) == SQL_SUCCESS || (rc) == SQL_SUCCESS_WITH_INFO) << GetOdbcError(handle, type)

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

inline const char* kConnStr = "Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;";
