#pragma once

<<<<<<<< HEAD:include/ydb-cpp-sdk/client/table/query_stats/stats.h
#include <ydb-cpp-sdk/client/query/stats.h>
========
#include <src/client/ydb_query/stats.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_table/query_stats/stats.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_table/query_stats/stats.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

class TDuration;

namespace Ydb {
    namespace TableStats {
        class QueryStats;
    }

    namespace Table {
        class QueryStatsCollection;
    }
}

namespace NYdb {

class TProtoAccessor;

namespace NScripting {

class TScriptingClient;
class TYqlResultPartIterator;

} // namespace NScripting

namespace NTable {

enum class ECollectQueryStatsMode {
    None = 0,  // Stats collection is disabled
    Basic = 1, // Aggregated stats of reads, updates and deletes per table
    Full = 2,   // Add per-stage execution profile and query plan on top of Basic mode
    Profile = 3   // Detailed execution stats including stats for individual tasks and channels
};

using TQueryStats = NQuery::TExecStats;

std::optional<ECollectQueryStatsMode> ParseQueryStatsMode(std::string_view statsMode);

std::string_view QueryStatsModeToString(ECollectQueryStatsMode statsMode);

} // namespace NTable
} // namespace NYdb
