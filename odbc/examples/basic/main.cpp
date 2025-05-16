#include <sql.h>
#include <sqlext.h>

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdint>

void PrintOdbcError(SQLSMALLINT handleType, SQLHANDLE handle) {
    SQLCHAR sqlState[6] = {0};
    SQLINTEGER nativeError = 0;
    SQLCHAR message[256] = {0};
    SQLSMALLINT textLength = 0;
    SQLGetDiagRec(handleType, handle, 1, sqlState, &nativeError, message, sizeof(message), &textLength);
    std::cerr << "ODBC error: [" << sqlState << "] " << message << std::endl;
}

int main() {
    SQLHENV henv = nullptr;
    SQLHDBC hdbc = nullptr;
    SQLHSTMT hstmt = nullptr;
    SQLRETURN ret;

    std::cout << "1. Allocating environment handle" << std::endl;
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating environment handle" << std::endl;
        return 1;
    }
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    std::cout << "2. Allocating connection handle" << std::endl;
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating connection handle" << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    std::cout << "3. Building connection string" << std::endl;
    std::string connStr = "Driver=" ODBC_DRIVER_PATH ";Endpoint=localhost:2136;Database=/local;";
    SQLCHAR outConnStr[1024] = {0};
    SQLSMALLINT outConnStrLen = 0;

    std::cout << "4. Connecting with SQLDriverConnect" << std::endl;
    ret = SQLDriverConnect(hdbc, NULL, (SQLCHAR*)connStr.c_str(), SQL_NTS,
                          outConnStr, sizeof(outConnStr), &outConnStrLen, SQL_DRIVER_COMPLETE);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error connecting with SQLDriverConnect" << std::endl;
        PrintOdbcError(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    std::cout << "5. Allocating statement handle" << std::endl;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error allocating statement handle" << std::endl;
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    std::cout << "6. Executing query" << std::endl;
    SQLCHAR query[] = R"(
        DECLARE $p1 AS Int64;
        SELECT $p1 + 1, 'test1' as String;
        SELECT $p1 + 2, 'test2' as String;
        SELECT $p1 + 3, 'test3' as String;
        SELECT $p1 + 4, 'test4' as String;
        SELECT $p1 + 5, 'test5' as String;
        SELECT $p1 + 6, 'test6' as String;
        SELECT $p1 + 7, 'test7' as String;
        SELECT $p1 + 8, 'test8' as String;
        SELECT $p1 + 9, 'test9' as String;
    )";

    int64_t paramValue = 42;
    SQLLEN paramInd = 0;
    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &paramValue, 0, &paramInd);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error binding parameter" << std::endl;
        PrintOdbcError(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    ret = SQLExecDirect(hstmt, query, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Error executing query" << std::endl;
        PrintOdbcError(SQL_HANDLE_STMT, hstmt);
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    std::cout << "7. Fetching result" << std::endl;

    SQLLEN ind = 0;
    int value1 = 0;
    if (SQLBindCol(hstmt, 1, SQL_C_SLONG, &value1, 0, &ind) != SQL_SUCCESS) {
        std::cerr << "Error binding column 1" << std::endl;
        PrintOdbcError(SQL_HANDLE_STMT, hstmt);
        return 1;
    }

    SQLCHAR value2[1024] = {0};
    if (SQLBindCol(hstmt, 2, SQL_C_CHAR, &value2, 1024, &ind) != SQL_SUCCESS) {
        std::cerr << "Error binding column 2" << std::endl;
        PrintOdbcError(SQL_HANDLE_STMT, hstmt);
        return 1;
    }

    while ((ret = SQLFetch(hstmt)) == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        if (ret != SQL_SUCCESS) {
            std::cerr << "Error fetching result" << std::endl;
            PrintOdbcError(SQL_HANDLE_STMT, hstmt);
            return 1;
        }

        std::cout << "Result column 1: " << value1 << std::endl;
        std::cout << "Result column 2: " << value2 << std::endl;

        std::cout << "--------------------------------" << std::endl;
    }

    std::cout << "8. Cleaning up" << std::endl;
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return 0;
}
