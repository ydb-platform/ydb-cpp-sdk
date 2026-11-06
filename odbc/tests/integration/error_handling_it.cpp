#include "test_utils.h"

TEST(ErrorHandling, GetDiagRecAfterError) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    SQLRETURN rc = SQLConnect(dbc, (SQLCHAR*)"NONEXISTENT_DSN", SQL_NTS,
                             (SQLCHAR*)"", SQL_NTS, (SQLCHAR*)"", SQL_NTS);
    ASSERT_EQ(rc, SQL_ERROR);
    SQLCHAR sqlState[6];
    SQLINTEGER nativeError;
    SQLCHAR msg[256];
    SQLSMALLINT msgLen;
    SQLRETURN diagRc = SQLGetDiagRec(SQL_HANDLE_DBC, dbc, 1, sqlState, &nativeError,
                                     msg, sizeof(msg), &msgLen);
    ASSERT_TRUE(diagRc == SQL_SUCCESS || diagRc == SQL_SUCCESS_WITH_INFO);
    const size_t copiedLen = std::strlen(reinterpret_cast<char*>(msg));
    ASSERT_EQ(msg[copiedLen], static_cast<SQLCHAR>(0));
    if (diagRc == SQL_SUCCESS_WITH_INFO) {
        ASSERT_GE(static_cast<size_t>(msgLen), copiedLen);
    } else {
        ASSERT_EQ(static_cast<size_t>(msgLen), copiedLen);
    }
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ErrorHandling, GetDiagRecMultipleErrors) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    
    SQLExecDirect(stmt, (SQLCHAR*)"INVALID SYNTAX HERE", SQL_NTS);
    
    SQLINTEGER numRecs;
    SQLGetDiagField(SQL_HANDLE_STMT, stmt, 0, SQL_DIAG_NUMBER, &numRecs, 0, nullptr);
    
    for (SQLINTEGER i = 1; i <= numRecs; ++i) {
        SQLCHAR sqlState[6];
        SQLINTEGER nativeError;
        SQLCHAR msg[256];
        SQLSMALLINT msgLen;
        SQLRETURN rc = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, static_cast<SQLSMALLINT>(i), sqlState, &nativeError,
                                     msg, sizeof(msg), &msgLen);
        ASSERT_TRUE(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
    }
    
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ErrorHandling, GetDiagFieldState) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"SELECT invalid_column FROM nonexistent_table", SQL_NTS);
    SQLCHAR sqlState[6];
    SQLRETURN rc = SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_SQLSTATE,
                                     sqlState, sizeof(sqlState), nullptr);
    ASSERT_TRUE(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ErrorHandling, GetDiagFieldNativeError) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"SELECT * FROM nonexistent_table", SQL_NTS);
    SQLINTEGER nativeError;
    SQLRETURN rc = SQLGetDiagField(SQL_HANDLE_STMT, stmt, 1, SQL_DIAG_NATIVE,
                                     &nativeError, sizeof(nativeError), nullptr);
    ASSERT_TRUE(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}


TEST(ErrorHandling, SuccessWithInfo) {
    SQLHENV env;
    SQLHDBC dbc;
    AllocEnv(&env);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc), SQL_SUCCESS);
    SQLCHAR outStr[10] = {};
    SQLSMALLINT outLen = -1;
    SQLRETURN rc = SQLDriverConnect(dbc, nullptr, (SQLCHAR*)kConnStr, SQL_NTS,
                                    outStr, sizeof(outStr), &outLen, SQL_DRIVER_NOPROMPT);
    ASSERT_EQ(rc, SQL_SUCCESS_WITH_INFO);
#ifdef ODBC_TEST_IODBC
    // iODBC 3.52 overwrites the driver's required output length with the
    // length of the truncated buffer and does not expose the driver's 01004
    // diagnostic record. The return code and truncated contents are retained.
    EXPECT_EQ(outLen, static_cast<SQLSMALLINT>(sizeof(outStr) - 1));
#else
    EXPECT_EQ(outLen, static_cast<SQLSMALLINT>(std::strlen(kConnStr)));
#endif
    EXPECT_EQ(std::string(reinterpret_cast<char*>(outStr)), std::string(kConnStr, sizeof(outStr) - 1));
#ifdef ODBC_TEST_IODBC
    SQLCHAR sqlState[6] = {};
    SQLINTEGER nativeError = 0;
    SQLCHAR message[256] = {};
    SQLSMALLINT messageLength = 0;
    EXPECT_EQ(SQLGetDiagRec(SQL_HANDLE_DBC, dbc, 1, sqlState, &nativeError,
                            message, sizeof(message), &messageLength),
              SQL_NO_DATA);
#else
    EXPECT_TRUE(SqlStatePrefix(GetOdbcError(dbc, SQL_HANDLE_DBC), "01004"));
#endif
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}

TEST(ErrorHandling, ClearErrors) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    AllocEnvAndConnect(&env, &dbc);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt), SQL_SUCCESS);
    SQLExecDirect(stmt, (SQLCHAR*)"SELECT * FROM nonexistent_table", SQL_NTS);
    SQLINTEGER numRecs1;
    SQLGetDiagField(SQL_HANDLE_STMT, stmt, 0, SQL_DIAG_NUMBER, &numRecs1, 0, nullptr);
    ASSERT_GT(numRecs1, 0);
    SQLExecDirect(stmt, (SQLCHAR*)"SELECT 1", SQL_NTS);
    SQLINTEGER numRecs2;
    SQLGetDiagField(SQL_HANDLE_STMT, stmt, 0, SQL_DIAG_NUMBER, &numRecs2, 0, nullptr);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
}
