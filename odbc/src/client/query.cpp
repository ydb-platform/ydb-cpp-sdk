#include "client/query.h"

#include <ydb-cpp-sdk/client/query/client.h>

#include <string>

extern "C" {

void* YDB_CreateQueryClient(void* driver) {
    if (!driver) return nullptr;
    
    try {
        auto* ydb_driver = static_cast<NYdb::TDriver*>(driver);
        auto* query_client = new NYdb::NQuery::TQueryClient(*ydb_driver);
        return static_cast<void*>(query_client);
    } catch (...) {
        return nullptr;
    }
}

void YDB_DestroyQueryClient(void* query_client) {
    if (query_client) {
        auto* client = static_cast<NYdb::NQuery::TQueryClient*>(query_client);
        delete client;
    }
}

int YDB_ExecuteQuery(void* query_client, const char* query, void** result) {
    if (!query_client || !query || !result) {
        return 0;
    }

    try {
        auto* client = static_cast<NYdb::NQuery::TQueryClient*>(query_client);
        auto executeResult = client->ExecuteQuery(std::string(query, strlen(query)), NYdb::NQuery::TTxControl::NoTx()).GetValueSync();

        if (!executeResult.IsSuccess()) {
            return 0;
        }

        *result = reinterpret_cast<void*>(new NYdb::NQuery::TExecuteQueryResult(executeResult));

        return 1;
    } catch (...) {
        return 0;
    }
}

void YDB_FreeExecuteQueryResult(void* result) {
    if (result) {
        auto* executeResult = reinterpret_cast<NYdb::NQuery::TExecuteQueryResult*>(result);
        delete executeResult;
    }
}

} // extern "C"
