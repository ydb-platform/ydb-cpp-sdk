#pragma once

#include <sql.h>
#include <sqlext.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Структура для хранения информации о драйвере
typedef struct {
    char name[256];
    char version[64];
    char description[1024];
} YDB_DRIVER_INFO;

// Структура для хранения состояния соединения
typedef struct {
    void* ydb_driver;
    void* query_client;
    int connected;
} YDB_CONNECTION;

// Структура для хранения состояния оператора
typedef struct {
    YDB_CONNECTION* connection;
    void* query_client;
    void* result;
    size_t current_row;
} YDB_STATEMENT;

// Структура для хранения дескриптора
typedef struct {
    void** descriptors;
    size_t descriptors_size;
} YDB_DESCRIPTOR;

// Функции драйвера
SQLRETURN YDB_SQLGetInfo(SQLSMALLINT InfoType, SQLPOINTER InfoValue,
                        SQLSMALLINT BufferLength, SQLSMALLINT* StringLength);

SQLRETURN YDB_SQLConnect(SQLHDBC ConnectionHandle, SQLCHAR* ServerName,
                        SQLSMALLINT NameLength1, SQLCHAR* UserName,
                        SQLSMALLINT NameLength2, SQLCHAR* Authentication,
                        SQLSMALLINT NameLength3);

SQLRETURN YDB_SQLDriverConnect(SQLHDBC ConnectionHandle, SQLHWND WindowHandle,
                             SQLCHAR* InConnectionString, SQLSMALLINT StringLength1,
                             SQLCHAR* OutConnectionString, SQLSMALLINT BufferLength,
                             SQLSMALLINT* StringLength2, SQLUSMALLINT DriverCompletion);

// Функции соединения
SQLRETURN YDB_SQLDisconnect(SQLHDBC ConnectionHandle);

SQLRETURN YDB_SQLGetConnectionInfo(SQLHDBC ConnectionHandle, SQLSMALLINT InfoType,
                                 SQLPOINTER InfoValue, SQLSMALLINT BufferLength,
                                 SQLSMALLINT* StringLength);

SQLRETURN YDB_SQLAllocHandle(SQLSMALLINT HandleType, SQLHANDLE InputHandle,
                           SQLHANDLE* OutputHandle);

SQLRETURN YDB_SQLFreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle);

// Функции оператора
SQLRETURN YDB_SQLExecDirect(SQLHSTMT StatementHandle, SQLCHAR* StatementText,
                          SQLINTEGER TextLength);

SQLRETURN YDB_SQLPrepare(SQLHSTMT StatementHandle, SQLCHAR* StatementText,
                        SQLINTEGER TextLength);

SQLRETURN YDB_SQLExecute(SQLHSTMT StatementHandle);

SQLRETURN YDB_SQLFetch(SQLHSTMT StatementHandle);

SQLRETURN YDB_SQLCloseCursor(SQLHSTMT StatementHandle);

// Функции дескриптора
SQLRETURN YDB_SQLGetDescField(SQLHDESC DescriptorHandle, SQLSMALLINT RecNumber,
                            SQLSMALLINT FieldIdentifier, SQLPOINTER Value,
                            SQLINTEGER BufferLength, SQLINTEGER* StringLength);

SQLRETURN YDB_SQLSetDescField(SQLHDESC DescriptorHandle, SQLSMALLINT RecNumber,
                            SQLSMALLINT FieldIdentifier, SQLPOINTER Value,
                            SQLINTEGER BufferLength);

#ifdef __cplusplus
}
#endif
