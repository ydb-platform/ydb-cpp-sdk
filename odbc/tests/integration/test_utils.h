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

inline const char* kConnStr = "Driver=" ODBC_DRIVER_PATH ";Server=localhost:2136;Database=/local;";

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

// ============================================================================
// Type and Parameter Utilities
// ============================================================================

// Bind integer parameter and return result
inline SQLRETURN BindIntParam(SQLHSTMT stmt, SQLUSMALLINT paramNum, SQLINTEGER* value) {
    return SQLBindParameter(stmt, paramNum, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, value, 0, nullptr);
}

inline SQLRETURN BindInt64Param(SQLHSTMT stmt, SQLUSMALLINT paramNum, SQLBIGINT* value) {
    return SQLBindParameter(stmt, paramNum, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, value, 0, nullptr);
}

inline SQLRETURN BindStringParam(SQLHSTMT stmt, SQLUSMALLINT paramNum, char* value, SQLLEN len) {
    SQLLEN indicator = (len >= 0) ? len : SQL_NTS;
    return SQLBindParameter(stmt, paramNum, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 
                            0, 0, value, (indicator == SQL_NTS) ? 0 : indicator, &indicator);
}

inline SQLRETURN BindDoubleParam(SQLHSTMT stmt, SQLUSMALLINT paramNum, double* value) {
    return SQLBindParameter(stmt, paramNum, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, value, 0, nullptr);
}

inline SQLRETURN BindNullParam(SQLHSTMT stmt, SQLUSMALLINT paramNum, SQLINTEGER* placeholder) {
    static SQLLEN nullIndicator = SQL_NULL_DATA;
    return SQLBindParameter(stmt, paramNum, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 
                            0, 0, placeholder, 0, &nullIndicator);
}

// Fetch and verify integer result
inline SQLINTEGER FetchIntResult(SQLHSTMT stmt, SQLUSMALLINT colNum = 1) {
    SQLINTEGER result = 0;
    SQLLEN indicator = 0;
    SQLBindCol(stmt, colNum, SQL_C_LONG, &result, 0, &indicator);
    SQLFetch(stmt);
    return result;
}

inline SQLBIGINT FetchInt64Result(SQLHSTMT stmt, SQLUSMALLINT colNum = 1) {
    SQLBIGINT result = 0;
    SQLLEN indicator = 0;
    SQLBindCol(stmt, colNum, SQL_C_SBIGINT, &result, 0, &indicator);
    SQLFetch(stmt);
    return result;
}

inline double FetchDoubleResult(SQLHSTMT stmt, SQLUSMALLINT colNum = 1) {
    double result = 0.0;
    SQLLEN indicator = 0;
    SQLBindCol(stmt, colNum, SQL_C_DOUBLE, &result, 0, &indicator);
    SQLFetch(stmt);
    return result;
}

inline std::string FetchStringResult(SQLHSTMT stmt, SQLUSMALLINT colNum = 1, size_t maxLen = 256) {
    std::string result(maxLen, '\0');
    SQLLEN indicator = 0;
    SQLBindCol(stmt, colNum, SQL_C_CHAR, &result[0], maxLen, &indicator);
    SQLFetch(stmt);
    if (indicator > 0 && indicator != SQL_NULL_DATA) {
        result.resize(indicator);
    }
    return result;
}

inline bool IsNullResult(SQLHSTMT stmt, SQLUSMALLINT colNum = 1) {
    SQLINTEGER dummy;
    SQLLEN indicator = 0;
    SQLBindCol(stmt, colNum, SQL_C_LONG, &dummy, 0, &indicator);
    SQLFetch(stmt);
    return indicator == SQL_NULL_DATA;
}
