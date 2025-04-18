#include "ydb_odbc.h"
#include "client/query.h"
#include <string.h>
#include <stdlib.h>

SQLRETURN YDB_SQLExecDirect(SQLHSTMT StatementHandle, SQLCHAR* StatementText,
                            SQLINTEGER TextLength) {
    YDB_STATEMENT* stmt = (YDB_STATEMENT*)StatementHandle;
    if (!stmt || !stmt->connection || !stmt->connection->connected || !StatementText) {
        return SQL_ERROR;
    }

    // Если текст запроса не указан, используем длину строки
    if (TextLength == SQL_NTS) {
        TextLength = strlen((char*)StatementText);
    }

    // Создаем клиент запросов, если его еще нет
    if (!stmt->query_client) {
        stmt->query_client = YDB_CreateQueryClient(stmt->connection->ydb_driver);
        if (!stmt->query_client) {
            return SQL_ERROR;
        }
    }

    // Освобождаем предыдущий результат, если он есть
    if (stmt->result) {
        YDB_FreeExecuteQueryResult(stmt->result);
        stmt->result = NULL;
    }

    // Выполняем запрос
    if (!YDB_ExecuteQuery(stmt->query_client, (char*)StatementText, &stmt->result)) {
        return SQL_ERROR;
    }

    stmt->current_row = 0;
    return SQL_SUCCESS;
}

SQLRETURN YDB_SQLPrepare(SQLHSTMT StatementHandle, SQLCHAR* StatementText,
                         SQLINTEGER TextLength) {
    // YDB не требует предварительной подготовки запросов
    // Просто сохраняем текст запроса для последующего выполнения
    YDB_STATEMENT* stmt = (YDB_STATEMENT*)StatementHandle;
    if (!stmt || !StatementText) {
        return SQL_ERROR;
    }

    // Если текст запроса не указан, используем длину строки
    if (TextLength == SQL_NTS) {
        TextLength = strlen((char*)StatementText);
    }

    // TODO: Сохранить текст запроса для последующего выполнения

    return SQL_SUCCESS;
}

SQLRETURN YDB_SQLExecute(SQLHSTMT StatementHandle) {
    // В YDB все запросы выполняются сразу
    // Эта функция просто вызывает SQLExecDirect с сохраненным текстом запроса
    YDB_STATEMENT* stmt = (YDB_STATEMENT*)StatementHandle;
    if (!stmt) {
        return SQL_ERROR;
    }

    // TODO: Выполнить сохраненный запрос

    return SQL_SUCCESS;
}

SQLRETURN YDB_SQLFetch(SQLHSTMT StatementHandle) {
    YDB_STATEMENT* stmt = (YDB_STATEMENT*)StatementHandle;
    if (!stmt || !stmt->result) {
        return SQL_ERROR;
    }

    // TODO: Преобразовать данные текущей строки в формат ODBC

    stmt->current_row++;
    return SQL_SUCCESS;
}

SQLRETURN YDB_SQLCloseCursor(SQLHSTMT StatementHandle) {
    YDB_STATEMENT* stmt = (YDB_STATEMENT*)StatementHandle;
    if (!stmt) {
        return SQL_ERROR;
    }

    if (stmt->result) {
        YDB_FreeExecuteQueryResult(stmt->result);
        stmt->result = NULL;
    }

    stmt->current_row = 0;
    return SQL_SUCCESS;
} 