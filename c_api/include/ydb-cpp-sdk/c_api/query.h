#pragma once

#include <stddef.h>

#include "driver.h"
#include "result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TYdbQueryClientImpl TYdbQueryClient;
typedef struct TYdbQueryResultImpl TYdbQueryResult;

typedef enum {
    YDB_QUERY_CLIENT_OK,
    YDB_QUERY_CLIENT_ERROR,
} EYdbQueryClientError;

typedef enum {
    YDB_QUERY_RESULT_OK,
    YDB_QUERY_RESULT_ERROR,
} EYdbQueryResultError;

// Создание и уничтожение клиента запросов
TYdbQueryClient* YdbCreateQueryClient(TYdbDriver* driver);
void YdbDestroyQueryClient(TYdbQueryClient* queryClient);

// Выполнение запроса
TYdbQueryResult* YdbExecuteQuery(TYdbQueryClient* queryClient, const char* query);
void YdbDestroyQueryResult(TYdbQueryResult* result);

// Получение результата запроса
int YdbGetQueryResultSetsCount(TYdbQueryResult* result);
TYdbResultSet* YdbGetQueryResultSet(TYdbQueryResult* result, size_t index);

#ifdef __cplusplus
}
#endif
