#pragma once

#include <src/client/impl/ydb_internal/internal_header.h>

#include <ydb/public/api/protos/ydb_table.pb.h>
#include <ydb-cpp-sdk/client/table/query_stats/stats.h>

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
