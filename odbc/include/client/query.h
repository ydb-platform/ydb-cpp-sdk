#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* YDB_CreateQueryClient(void* driver);
void YDB_DestroyQueryClient(void* query_client);

int YDB_ExecuteQuery(void* query_client, const char* query, void** result);
void YDB_FreeExecuteQueryResult(void* result);

#ifdef __cplusplus
}
#endif
