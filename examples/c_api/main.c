#include <ydb-cpp-sdk/c_api/query.h>

#include <stdio.h>

#include <inttypes.h>

int main() {
    TYdbDriver* driver = YdbCreateDriver("grpc://localhost:2136/?database=/local");

    TYdbQueryClient* query = YdbCreateQueryClient(driver);

    TYdbQueryResult* result = YdbExecuteQuery(query, "SELECT 1");

    int resultSetsCount = YdbGetQueryResultSetsCount(result);
    for (int i = 0; i < resultSetsCount; i++) {
        TYdbResultSet* resultSet = YdbGetQueryResultSet(result, i);
        int rowsCount = YdbGetRowsCount(resultSet);
        for (int j = 0; j < rowsCount; j++) {
            TYdbValue* value = YdbGetValueByIndex(resultSet, j, 0);

            EYdbPrimitiveType primitiveType = YdbGetPrimitiveType(value);
            if (primitiveType == YDB_PRIMITIVE_TYPE_INT32) {
                int32_t int32Value;
                YdbGetInt32(value, &int32Value);
                printf("%" PRId32 "\n", int32Value);
            } else {
                printf("Unknown primitive type\n");
            }
            YdbDestroyValue(value);
        }
    }

    YdbDestroyQueryResult(result);
    YdbDestroyQueryClient(query);
    YdbDestroyDriver(driver);
    return 0;
}
