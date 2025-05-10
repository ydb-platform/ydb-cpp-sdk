#include <ydb-cpp-sdk/c_api/query.h>

#include <ydb-cpp-sdk/client/query/client.h>

#include "impl/driver_impl.h" // NOLINT
#include "impl/result_impl.h" // NOLINT

#include <string>

struct TYdbQueryClientImpl {
    EYdbQueryClientError errorCode;
    std::string errorMessage;

    std::optional<NYdb::NQuery::TQueryClient> client;
};

struct TYdbQueryResultImpl {
    EYdbQueryResultError errorCode;
    std::string errorMessage;

    std::optional<NYdb::NQuery::TExecuteQueryResult> result;
};

extern "C" {

TYdbQueryClient* YdbCreateQueryClient(TYdbDriver* driver) {
    try {
        if (!driver || !driver->driver.has_value()) {
            return new TYdbQueryClient{
                YDB_QUERY_CLIENT_ERROR,
                "Invalid driver"
            };
        }

        try {
            return new TYdbQueryClient{
                YDB_QUERY_CLIENT_OK,
                "",
                NYdb::NQuery::TQueryClient(*driver->driver)
            };
        } catch (const std::exception& e) {
            return new TYdbQueryClient{
                YDB_QUERY_CLIENT_ERROR,
                e.what()
            };
        }
    } catch (...) {
        return nullptr;
    }
}

void YdbDestroyQueryClient(TYdbQueryClient* queryClient) {
    if (queryClient) {
        delete queryClient;
    }
}

TYdbQueryResult* YdbExecuteQuery(TYdbQueryClient* queryClient, const char* query) {
    try {
        if (!queryClient || !queryClient->client.has_value()) {
            return new TYdbQueryResult{
                YDB_QUERY_RESULT_ERROR,
                "Invalid query client"
            };
        }

        if (!query) {
            return new TYdbQueryResult{
                YDB_QUERY_RESULT_ERROR,
                "Invalid query"
            };
        }

        try {
            auto client = *queryClient->client;
            auto executeResult = client.ExecuteQuery(
                std::string(query),
                NYdb::NQuery::TTxControl::NoTx()
            ).GetValueSync();

            if (!executeResult.IsSuccess()) {
                return new TYdbQueryResult{
                    YDB_QUERY_RESULT_ERROR,
                    "Query execution failed: " + executeResult.GetIssues().ToString()
                };
            }

            return new TYdbQueryResult{
                YDB_QUERY_RESULT_OK,
                "",
                executeResult
            };
        } catch (const std::exception& e) {
            return new TYdbQueryResult{
                YDB_QUERY_RESULT_ERROR,
                e.what()
            };
        }
    } catch (...) {
        return nullptr;
    }
}

void YdbDestroyQueryResult(TYdbQueryResult* result) {
    if (result) {
        delete result;
    }
}

TYdbResultSet* YdbGetQueryResultSet(TYdbQueryResult* result, size_t index) {
    try {
        if (!result || !result->result.has_value()) {
            return nullptr;
        }

        try {
            return new TYdbResultSet{
                YDB_RESULT_SET_OK,
                "",
                result->result->GetResultSet(index)
            };
        } catch (const std::exception& e) {
            return new TYdbResultSet{
                YDB_RESULT_SET_ERROR,
                e.what()
            };
        }
    } catch (...) {
        return nullptr;
    }
}

int YdbGetQueryResultSetsCount(TYdbQueryResult* result) {
    if (!result || !result->result.has_value()) {
        return -1;
    }
    return result->result->GetResultSets().size();
}

}
