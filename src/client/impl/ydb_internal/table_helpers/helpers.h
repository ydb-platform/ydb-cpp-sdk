#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

#include <src/api/protos/ydb_table.pb.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/client/table/query_stats/stats.h>
=======
#include <src/client/ydb_table/query_stats/stats.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/client/table/query_stats/stats.h>
>>>>>>> origin/main

namespace NYdb {

inline Ydb::Table::QueryStatsCollection::Mode GetStatsCollectionMode(std::optional<NTable::ECollectQueryStatsMode> mode) {
    if (mode.has_value()) {
        switch (*mode) {
            case NTable::ECollectQueryStatsMode::None:
                return Ydb::Table::QueryStatsCollection::STATS_COLLECTION_NONE;
            case NTable::ECollectQueryStatsMode::Basic:
                return Ydb::Table::QueryStatsCollection::STATS_COLLECTION_BASIC;
            case NTable::ECollectQueryStatsMode::Full:
                return Ydb::Table::QueryStatsCollection::STATS_COLLECTION_FULL;
            case NTable::ECollectQueryStatsMode::Profile:
                return Ydb::Table::QueryStatsCollection::STATS_COLLECTION_PROFILE;
            default:
                break;
        }
    }

    return Ydb::Table::QueryStatsCollection::STATS_COLLECTION_UNSPECIFIED;
}

} // namespace NYdb
