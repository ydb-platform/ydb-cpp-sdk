#include "kv_common.h"

#include <ydb-cpp-sdk/client/table/table.h>

using namespace NYdb;
using namespace NYdb::NTable;

int CreateTable(TDatabaseOptions& dbOptions) {
    TTableClient client(dbOptions.Driver);
    NYdb::NStatusHelpers::ThrowOnError(client.RetryOperationSync(
        [prefix = dbOptions.Prefix](TSession session) {
            auto desc = TTableBuilder()
                .AddNullableColumn("object_id_key", EPrimitiveType::Uint32)
                .AddNullableColumn("object_id", EPrimitiveType::Uint32)
                .AddNullableColumn("timestamp", EPrimitiveType::Uint64)
                .AddNullableColumn("payload", EPrimitiveType::Utf8)
                .SetPrimaryKeyColumns({"object_id_key", "object_id"})
                .Build();

            auto tableSettings = TCreateTableSettings()
                .PartitioningPolicy(TPartitioningPolicy().UniformPartitions(PartitionsCount))
                .CancelAfter(DefaultReactionTime)
                .ClientTimeout(DefaultReactionTime + TDuration::Seconds(5));

            return session.CreateTable(
                JoinPath(prefix, TableName),
                std::move(desc),
                std::move(tableSettings)
            ).ExtractValueSync();
        }));
    Cout << "Table created." << Endl;
    return EXIT_SUCCESS;
}
