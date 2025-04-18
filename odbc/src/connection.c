#include "ydb_odbc.h"
#include "client/driver.h"
#include "client/query.h"

#include <string.h>
#include <stdlib.h>

SQLRETURN YDB_SQLDisconnect(SQLHDBC ConnectionHandle) {
    YDB_CONNECTION* conn = (YDB_CONNECTION*)ConnectionHandle;
    if (!conn) {
        return SQL_ERROR;
    }

    if (conn->connected) {
        if (conn->query_client) {
            YDB_DestroyQueryClient(conn->query_client);
            conn->query_client = NULL;
        }
        
        if (conn->ydb_driver) {
            YDB_DestroyDriver(conn->ydb_driver);
            conn->ydb_driver = NULL;
        }
        
        conn->connected = 0;
    }

    return SQL_SUCCESS;
}

SQLRETURN YDB_SQLGetConnectionInfo(SQLHDBC ConnectionHandle, SQLSMALLINT InfoType,
                                 SQLPOINTER InfoValue, SQLSMALLINT BufferLength,
                                 SQLSMALLINT* StringLength) {
    YDB_CONNECTION* conn = (YDB_CONNECTION*)ConnectionHandle;
    if (!conn || !conn->connected) {
        return SQL_ERROR;
    }

    switch (InfoType) {
        case SQL_DATABASE_NAME:
            if (InfoValue && BufferLength > 0) {
                const char* dbName = "YDB";
                strncpy((char*)InfoValue, dbName, BufferLength - 1);
                if (StringLength) {
                    *StringLength = strlen(dbName);
                }
                return SQL_SUCCESS;
            }
            break;
            
        case SQL_SERVER_NAME:
            if (InfoValue && BufferLength > 0) {
                const char* serverName = "Yandex Database";
                strncpy((char*)InfoValue, serverName, BufferLength - 1);
                if (StringLength) {
                    *StringLength = strlen(serverName);
                }
                return SQL_SUCCESS;
            }
            break;
    }

    return SQL_ERROR;
}

SQLRETURN YDB_SQLAllocHandle(SQLSMALLINT HandleType, SQLHANDLE InputHandle,
                           SQLHANDLE* OutputHandle) {
    if (!OutputHandle) {
        return SQL_ERROR;
    }

    switch (HandleType) {
        case SQL_HANDLE_DBC:
            *OutputHandle = calloc(1, sizeof(YDB_CONNECTION));
            return SQL_SUCCESS;

        case SQL_HANDLE_STMT:
            *OutputHandle = calloc(1, sizeof(YDB_STATEMENT));
            return SQL_SUCCESS;

        case SQL_HANDLE_DESC:
            *OutputHandle = calloc(1, sizeof(YDB_DESCRIPTOR));
            return SQL_SUCCESS;

        default:
            return SQL_ERROR;
    }
}

SQLRETURN YDB_SQLFreeHandle(SQLSMALLINT HandleType, SQLHANDLE Handle) {
    if (!Handle) {
        return SQL_ERROR;
    }

    switch (HandleType) {
        case SQL_HANDLE_DBC:
            {
                YDB_CONNECTION* conn = (YDB_CONNECTION*)Handle;
                if (conn->connected) {
                    YDB_SQLDisconnect((SQLHDBC)conn);
                }
                free(conn);
            }
            return SQL_SUCCESS;

        case SQL_HANDLE_STMT:
            {
                YDB_STATEMENT* stmt = (YDB_STATEMENT*)Handle;
                if (stmt->result) {
                    YDB_FreeExecuteQueryResult(stmt->result);
                }
                if (stmt->query_client) {
                    YDB_DestroyQueryClient(stmt->query_client);
                }
                free(stmt);
            }
            return SQL_SUCCESS;

        case SQL_HANDLE_DESC:
            {
                YDB_DESCRIPTOR* desc = (YDB_DESCRIPTOR*)Handle;
                if (desc->descriptors) {
                    free(desc->descriptors);
                }
                free(desc);
            }
            return SQL_SUCCESS;

        default:
            return SQL_ERROR;
    }
} 