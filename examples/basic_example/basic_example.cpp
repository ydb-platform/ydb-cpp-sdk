#include "basic_example.h"

#include <ydb-cpp-sdk/client/retry/retry.h>

#include <util/string/cast.h>

#include <format>

using namespace NYdb;
using namespace NYdb::NQuery;

class TYdbErrorException : public yexception {
public:
    TYdbErrorException(const TStatus& status)
        : Status(status) {}

    TStatus Status;
};

static void ThrowOnError(const TStatus& status) {
    if (!status.IsSuccess()) {
        throw TYdbErrorException(status) << status;
    }
}

static void PrintStatus(const TStatus& status) {
    std::cerr << "Status: " << ToString(status.GetStatus()) << std::endl;
    std::cerr << status.GetIssues().ToString();
}

template <class T>
std::string OptionalToString(const std::optional<T>& opt) {
    if (opt.has_value()) {
        return std::to_string(opt.value());
    }
    return "(NULL)";
}

template <>
std::string OptionalToString<std::string>(const std::optional<std::string>& opt) {
    if (opt.has_value()) {
        return opt.value();
    }
    return "(NULL)";
}

///////////////////////////////////////////////////////////////////////////////

static void CreateTables(TQueryClient client, const std::string& path) {
    ThrowOnError(client.RetryQuerySync([path](TSession session) {
        auto query = std::format(R"(
            PRAGMA TablePathPrefix("{}");
            CREATE TABLE series (
                series_id Uint64,
                title Utf8,
                series_info Utf8,
                release_date Uint64,
                PRIMARY KEY (series_id)
            );
        )", path.c_str());
        return session.ExecuteQuery(query, TTxControl::NoTx()).GetValueSync();
    }));

    ThrowOnError(client.RetryQuerySync([path](TSession session) {
        auto query = std::format(R"(
            PRAGMA TablePathPrefix("{}");
            CREATE TABLE seasons (
                series_id Uint64,
                season_id Uint64,
                title Utf8,
                first_aired Uint64,
                last_aired Uint64,
                PRIMARY KEY (series_id, season_id)
            );
        )", path.c_str());
        return session.ExecuteQuery(query, TTxControl::NoTx()).GetValueSync();
    }));

    ThrowOnError(client.RetryQuerySync([path](TSession session) {
        auto query = std::format(R"(
            PRAGMA TablePathPrefix("{}");
            CREATE TABLE episodes (
                series_id Uint64,
                season_id Uint64,
                episode_id Uint64,
                title Utf8,
                air_date Uint64,
                PRIMARY KEY (series_id, season_id, episode_id)
            );
        )", path.c_str());
        return session.ExecuteQuery(query, TTxControl::NoTx()).GetValueSync();
    }));
}

///////////////////////////////////////////////////////////////////////////////

static void DropTables(TQueryClient client, const std::string& path) {
    ThrowOnError(client.RetryQuerySync([path](TSession session) {
        auto query = std::format(R"(
            PRAGMA TablePathPrefix("{}");
            DROP TABLE series;
        )", path.c_str());
        return session.ExecuteQuery(query, TTxControl::NoTx()).GetValueSync();
    }));

    ThrowOnError(client.RetryQuerySync([path](TSession session) {
        auto query = std::format(R"(
            PRAGMA TablePathPrefix("{}");
            DROP TABLE seasons;
        )", path.c_str());
        return session.ExecuteQuery(query, TTxControl::NoTx()).GetValueSync();
    }));
    
    ThrowOnError(client.RetryQuerySync([path](TSession session) {
        auto query = std::format(R"(
            PRAGMA TablePathPrefix("{}");
            DROP TABLE episodes;
        )", path.c_str());
        return session.ExecuteQuery(query, TTxControl::NoTx()).GetValueSync();
    }));
}

///////////////////////////////////////////////////////////////////////////////

void FillTableData(TQueryClient client, const std::string& path) {
    ThrowOnError(client.RetryQuerySync([path](TSession session) {
        auto query = std::format(R"(
        PRAGMA TablePathPrefix("{}");

        DECLARE $seriesData AS List<Struct<
            series_id: Uint64,
            title: Utf8,
            series_info: Utf8,
            release_date: Date>>;

        DECLARE $seasonsData AS List<Struct<
            series_id: Uint64,
            season_id: Uint64,
            title: Utf8,
            first_aired: Date,
            last_aired: Date>>;

        DECLARE $episodesData AS List<Struct<
            series_id: Uint64,
            season_id: Uint64,
            episode_id: Uint64,
            title: Utf8,
            air_date: Date>>;

        REPLACE INTO series
        SELECT
            series_id,
            title,
            series_info,
            CAST(release_date AS Uint16) AS release_date
        FROM AS_TABLE($seriesData);

        REPLACE INTO seasons
        SELECT
            series_id,
            season_id,
            title,
            CAST(first_aired AS Uint16) AS first_aired,
            CAST(last_aired AS Uint16) AS last_aired
        FROM AS_TABLE($seasonsData);

        REPLACE INTO episodes
        SELECT
            series_id,
            season_id,
            episode_id,
            title,
            CAST(air_date AS Uint16) AS air_date
        FROM AS_TABLE($episodesData);
        )", path.c_str());

        auto params = GetTablesDataParams();

        return session.ExecuteQuery(
            query,
            TTxControl::BeginTx(TTxSettings::SerializableRW()).CommitTx(),
            params).GetValueSync();
    }));
}

void SelectSimple(TQueryClient client, const std::string& path) {
    std::optional<TResultSet> resultSet;
    ThrowOnError(client.RetryQuerySync([&path, &resultSet](TSession session) {
        auto query = std::format(R"(
            PRAGMA TablePathPrefix("{}");

            SELECT series_id, title, CAST(CAST(release_date AS Date) AS String) AS release_date
            FROM series
            WHERE series_id = 1;
        )", path.c_str());

        auto txControl =
            // Begin new transaction with SerializableRW mode
            TTxControl::BeginTx(TTxSettings::SerializableRW())
            // Commit transaction at the end of the query
            .CommitTx();

        auto result = session.ExecuteQuery(query, txControl).GetValueSync();
        if (!result.IsSuccess()) {
            return result;
        }
        resultSet = result.GetResultSet(0);
        return result;
    }));

    TResultSetParser parser(*resultSet);
    if (parser.TryNextRow()) {
        std::cout << "> SelectSimple:" << std::endl << "Series"
                  << ", Id: " << OptionalToString(parser.ColumnParser("series_id").GetOptionalUint64())
                  << ", Title: " << OptionalToString(parser.ColumnParser("title").GetOptionalUtf8())
                  << ", Release date: " << OptionalToString(parser.ColumnParser("release_date").GetOptionalString())
                  << std::endl;
    }
}

void UpsertSimple(TQueryClient client, const std::string& path) {
    ThrowOnError(client.RetryQuerySync([path](TSession session) {
        auto query = std::format(R"(
            --!syntax_v1
            PRAGMA TablePathPrefix("{}");

            UPSERT INTO episodes (series_id, season_id, episode_id, title) VALUES
                (2, 6, 1, "TBD");
        )", path.c_str());

        return session.ExecuteQuery(query,
            TTxControl::BeginTx(TTxSettings::SerializableRW()).CommitTx()).GetValueSync();
    }));
}

void SelectWithParams(TQueryClient client, const std::string& path) {
    std::optional<TResultSet> resultSet;
    ThrowOnError(client.RetryQuerySync([&path, &resultSet](TSession session) {
        ui64 seriesId = 2;
        ui64 seasonId = 3;
        auto query = std::format(R"(
            --!syntax_v1
            PRAGMA TablePathPrefix("{}");

            DECLARE $seriesId AS Uint64;
            DECLARE $seasonId AS Uint64;

            SELECT sa.title AS season_title, sr.title AS series_title
            FROM seasons AS sa
            INNER JOIN series AS sr
            ON sa.series_id = sr.series_id
            WHERE sa.series_id = $seriesId AND sa.season_id = $seasonId;
        )", path.c_str());

        auto params = TParamsBuilder()
            .AddParam("$seriesId")
                .Uint64(seriesId)
                .Build()
            .AddParam("$seasonId")
                .Uint64(seasonId)
                .Build()
            .Build();

        auto result = session.ExecuteQuery(
            query,
            TTxControl::BeginTx(TTxSettings::SerializableRW()).CommitTx(),
            params).GetValueSync();
        
        if (!result.IsSuccess()) {
            return result;
        }
        resultSet = result.GetResultSet(0);
        return result;
    }));

    TResultSetParser parser(*resultSet);
    if (parser.TryNextRow()) {
        std::cout << "> SelectWithParams:" << std::endl << "Season"
                  << ", Title: " << OptionalToString(parser.ColumnParser("season_title").GetOptionalUtf8())
                  << ", Series title: " << OptionalToString(parser.ColumnParser("series_title").GetOptionalUtf8())
                  << std::endl;
    }
}

void MultiStep(TQueryClient client, const std::string& path) {
    std::optional<TResultSet> resultSet;
    ThrowOnError(client.RetryQuerySync([&path, &resultSet](TSession session) {
        ui64 seriesId = 2;
        ui64 seasonId = 5;
        auto query1 = std::format(R"(
            --!syntax_v1
            PRAGMA TablePathPrefix("{}");

            DECLARE $seriesId AS Uint64;
            DECLARE $seasonId AS Uint64;

            SELECT first_aired AS from_date FROM seasons
            WHERE series_id = $seriesId AND season_id = $seasonId;
        )", path.c_str());

        auto params1 = TParamsBuilder()
            .AddParam("$seriesId")
                .Uint64(seriesId)
                .Build()
            .AddParam("$seasonId")
                .Uint64(seasonId)
                .Build()
            .Build();

        // Execute first query to get the required values to the client.
        // Transaction control settings don't set CommitTx flag to keep transaction active
        // after query execution.
        auto result = session.ExecuteQuery(
            query1,
            TTxControl::BeginTx(TTxSettings::SerializableRW()),
            params1);

        auto resultValue = result.GetValueSync();

        if (!resultValue.IsSuccess()) {
            return resultValue;
        }

        // Get active transaction id
        auto tx = resultValue.GetTransaction();

        TResultSetParser parser(resultValue.GetResultSet(0));
        parser.TryNextRow();
        auto date = parser.ColumnParser("from_date").GetOptionalUint64();

        // Perform some client logic on returned values
        auto userFunc = [] (const TInstant fromDate) {
            return fromDate + TDuration::Days(15);
        };

        TInstant fromDate = TInstant::Days(*date);
        TInstant toDate = userFunc(fromDate);

        // Construct next query based on the results of client logic
        auto query2 = std::format(R"(
            --!syntax_v1
            PRAGMA TablePathPrefix("{}");

            DECLARE $seriesId AS Uint64;
            DECLARE $fromDate AS Uint64;
            DECLARE $toDate AS Uint64;

            SELECT season_id, episode_id, title, air_date FROM episodes
            WHERE series_id = $seriesId AND air_date >= $fromDate AND air_date <= $toDate;
        )", path.c_str());

        auto params2 = TParamsBuilder()
            .AddParam("$seriesId")
                .Uint64(seriesId)
                .Build()
            .AddParam("$fromDate")
                .Uint64(fromDate.Days())
                .Build()
            .AddParam("$toDate")
                .Uint64(toDate.Days())
                .Build()
            .Build();

        // Execute second query.
        // Transaction control settings continues active transaction (tx) and
        // commits it at the end of second query execution.
        auto result2 = session.ExecuteQuery(
            query2,
            TTxControl::Tx(tx->GetId()).CommitTx(),
            params2).GetValueSync();
        
        if (!result2.IsSuccess()) {
            return result2;
        }
        resultSet = result2.GetResultSet(0);
        return result2;
    }));

    TResultSetParser parser(*resultSet);
    std::cout << "> MultiStep:" << std::endl;
    while (parser.TryNextRow()) {
        auto airDate = TInstant::Days(*parser.ColumnParser("air_date").GetOptionalUint64());

        std::cout << "Episode " << OptionalToString(parser.ColumnParser("episode_id").GetOptionalUint64())
                  << ", Season: " << OptionalToString(parser.ColumnParser("season_id").GetOptionalUint64())
                  << ", Title: " << OptionalToString(parser.ColumnParser("title").GetOptionalUtf8())
                  << ", Air date: " << airDate.FormatLocalTime("%a %b %d, %Y")
                  << std::endl;
    }
}

void ExplicitTcl(TQueryClient client, const std::string& path) {
    // Show usage of explicit Begin/Commit transaction control calls.
    // In most cases it's better to use transaction control settings in ExecuteDataQuery calls instead
    // to avoid additional hops to YDB cluster and allow more efficient execution of queries.
    ThrowOnError(client.RetryQuerySync([&path](TQueryClient client) -> TStatus {
        auto airDate = TInstant::Now();
        auto session = client.GetSession().GetValueSync().GetSession();
        auto beginResult = session.BeginTransaction(TTxSettings::SerializableRW()).GetValueSync();
        if (!beginResult.IsSuccess()) {
            return beginResult;
        }

        // Get newly created transaction id
        auto tx = beginResult.GetTransaction();

        auto query = std::format(R"(
            --!syntax_v1
            PRAGMA TablePathPrefix("{}");

            DECLARE $airDate AS Date;

            UPDATE episodes SET air_date = CAST($airDate AS Uint16) WHERE title = "TBD";
        )", path.c_str());

        auto params = TParamsBuilder()
            .AddParam("$airDate")
                .Date(airDate)
                .Build()
            .Build();

        // Execute query.
        // Transaction control settings continues active transaction (tx)
        auto updateResult = session.ExecuteQuery(query,
            TTxControl::Tx(tx.GetId()),
            params).GetValueSync();

        if (!updateResult.IsSuccess()) {
            return updateResult;
        }
        // Commit active transaction (tx)
        return tx.Commit().GetValueSync();
    }));
}

void StreamQuerySelect(TQueryClient client, const std::string& path) {
    std::vector <TResultSet> resultSets;
    // WARNING: Do not use without RetryQuery!!!
    ThrowOnError(client.RetryQuerySync([path, &resultSets](TQueryClient client) -> TStatus {
        resultSets.clear();
        auto query = std::format(R"(
            --!syntax_v1
            PRAGMA TablePathPrefix("{}");

            DECLARE $series AS List<UInt64>;

            SELECT series_id, season_id, title, CAST(CAST(first_aired AS Date) AS String) AS first_aired
            FROM seasons
            WHERE series_id IN $series
            ORDER BY season_id;
        )", path.c_str());

        auto parameters = TParamsBuilder()
            .AddParam("$series")
            .BeginList()
                .AddListItem().Uint64(1)
                .AddListItem().Uint64(10)
            .EndList().Build()
            .Build();

        // Executes stream query
        auto resultStreamQuery = client.StreamExecuteQuery(query, TTxControl::NoTx(), parameters).GetValueSync();

        if (!resultStreamQuery.IsSuccess()) {
            return resultStreamQuery;
        }

        bool eos = false;

        while (!eos) {
            auto streamPart = resultStreamQuery.ReadNext().ExtractValueSync();

            if (!streamPart.IsSuccess()) {
                eos = true;
                if (!streamPart.EOS()) {
                    return streamPart;
                }
                continue;
            }

            if (streamPart.HasResultSet()) {
                auto rs = streamPart.ExtractResultSet();
                resultSets.push_back(rs);
            }
        }
        return TStatus(EStatus::SUCCESS, NYql::TIssues());
    }));
    
    std::cout << "> StreamQuery:" << std::endl;
    for (auto rs : resultSets) {
        TResultSetParser parser(rs);
        while (parser.TryNextRow()) {
            std::cout << "Season"
                      << ", SeriesId: " << OptionalToString(parser.ColumnParser("series_id").GetOptionalUint64())
                      << ", SeasonId: " << OptionalToString(parser.ColumnParser("season_id").GetOptionalUint64())
                      << ", Title: " << OptionalToString(parser.ColumnParser("title").GetOptionalUtf8())
                      << ", Air date: " << OptionalToString(parser.ColumnParser("first_aired").GetOptionalString())
                      << std::endl;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////

bool Run(const TDriver& driver, const std::string& path) {
    TQueryClient client(driver);

    try {
        CreateTables(client, path);

        FillTableData(client, path);

        SelectSimple(client, path);
        UpsertSimple(client, path);

        SelectWithParams(client, path);

        MultiStep(client, path);

        ExplicitTcl(client, path);

        StreamQuerySelect(client, path);

        DropTables(client, path);
    }
    catch (const TYdbErrorException& e) {
        std::cerr << "Execution failed due to fatal error:" << std::endl;
        PrintStatus(e.Status);
        return false;
    }

    return true;
}