-module(ydb_series_client).
-export([run/0, run/1, run_with_dsn/1]).

run() ->
    ConnectionString = "Driver=YDB;Endpoint=localhost:2136;Database=/local;",
    run(ConnectionString).

run(ConnectionString) when is_list(ConnectionString) ->
    io:format("=== ODBC YDB Series Example ===~n"),

    application:load(odbc),
    application:start(odbc),

    case odbc:connect(ConnectionString, [{tuple_format, list}]) of
        {ok, Ref} ->
            Result = run_example(Ref),
            odbc:disconnect(Ref),
            Result;
        {error, Reason} ->
            io:format("Connection failed: ~p~n", [Reason]),
            error
    end.

run_with_dsn(DSN) ->
    ConnectionString = lists:flatten(io_lib:format("DSN=~s;", [DSN])),
    run(ConnectionString).

run_example(Ref) ->
    try
        drop_tables(Ref),
        create_tables(Ref),
        fill_table_data(Ref),
        select_simple(Ref),
        upsert_simple(Ref),
        select_with_params(Ref),
        multistep(Ref),
        select_seasons_by_series(Ref),
        drop_tables(Ref),

        io:format("Completed successfully~n"),
        ok
    catch
        Class:Reason:Stacktrace ->
            io:format("~nError: ~p:~p~n", [Class, Reason]),
            io:format("Stacktrace: ~p~n", [Stacktrace]),
            error
    end.

create_tables(Ref) ->
    Tables = [
        {"CREATE TABLE series (
            series_id Uint64,
            title Utf8,
            series_info Utf8,
            release_date Uint64,
            PRIMARY KEY (series_id)
        );"},
        {"CREATE TABLE seasons (
            series_id Uint64,
            season_id Uint64,
            title Utf8,
            first_aired Uint64,
            last_aired Uint64,
            PRIMARY KEY (series_id, season_id)
        );"},
        {"CREATE TABLE episodes (
            series_id Uint64,
            season_id Uint64,
            episode_id Uint64,
            title Utf8,
            air_date Uint64,
            PRIMARY KEY (series_id, season_id, episode_id)
        );"}
    ],

    lists:foreach(fun({Query}) ->
        execute_update(Ref, Query)
    end, Tables).

fill_table_data(Ref) ->
    SeriesData = sample_data:series(),
    SeasonsData = sample_data:seasons(),
    EpisodesData = sample_data:episodes(),

    lists:foreach(fun(Row) ->
        [Id, Title, Info, Date] = Row,
        Query = io_lib:format(
            "UPSERT INTO series (series_id, title, series_info, release_date) VALUES (~p, \"~s\", \"~s\", ~p);",
            [Id, escape_string(Title), escape_string(Info), Date]
        ),
        execute_update(Ref, Query)
    end, SeriesData),

    lists:foreach(fun(Row) ->
        [SeriesId, SeasonId, Title, FirstAired, LastAired] = Row,
        Query = io_lib:format(
            "UPSERT INTO seasons (series_id, season_id, title, first_aired, last_aired) VALUES (~p, ~p, \"~s\", ~p, ~p);",
            [SeriesId, SeasonId, escape_string(Title), FirstAired, LastAired]
        ),
        execute_update(Ref, Query)
    end, SeasonsData),

    lists:foreach(fun(Row) ->
        [SeriesId, SeasonId, EpisodeId, Title, AirDate] = Row,
        Query = io_lib:format(
            "UPSERT INTO episodes (series_id, season_id, episode_id, title, air_date) VALUES (~p, ~p, ~p, \"~s\", ~p);",
            [SeriesId, SeasonId, EpisodeId, escape_string(Title), AirDate]
        ),
        execute_update(Ref, Query)
    end, EpisodesData),

    io:format("Inserted ~p series, ~p seasons, ~p episodes~n",
              [length(SeriesData), length(SeasonsData), length(EpisodesData)]).

select_simple(Ref) ->
    Query = "SELECT CAST(series_id AS Utf8) AS series_id, title, CAST(release_date AS Date) AS release_date FROM series WHERE series_id = 1;",
    Rows = selected_rows(Ref, select_simple, Query),
    lists:foreach(fun(Row) ->
        [Id, Title, ReleaseDate] = row_values(Row),
        io:format("Series: Id=~p, Title=~p, Release=~p~n", [Id, Title, ReleaseDate])
    end, Rows).

upsert_simple(Ref) ->
    Query = "UPSERT INTO episodes (series_id, season_id, episode_id, title) VALUES (2, 6, 1, \"TBD\");",
    execute_update(Ref, Query).

select_with_params(Ref) ->
    SeriesId = 2,
    SeasonId = 3,

    Query =
        "SELECT sa.title AS season_title, sr.title AS series_title "
        "FROM seasons AS sa INNER JOIN series AS sr ON sa.series_id = sr.series_id "
        "WHERE sa.series_id = CAST($p1 AS Uint64) AND sa.season_id = CAST($p2 AS Uint64);",
    Params = [{sql_integer, [SeriesId]}, {sql_integer, [SeasonId]}],

    Rows = selected_param_rows(Ref, select_with_params, Query, Params),
    lists:foreach(fun(Row) ->
        [SeasonTitle, SeriesTitle] = row_values(Row),
        io:format("Season: ~p (Series: ~p)~n", [SeasonTitle, SeriesTitle])
    end, Rows).

multistep(Ref) ->
    SeriesId = 2,
    SeasonId = 5,

    Query1 = io_lib:format(
        "SELECT CAST(first_aired AS Utf8) AS first_aired FROM seasons WHERE series_id = ~p AND season_id = ~p;",
        [SeriesId, SeasonId]
    ),

    [FirstAiredRow] = selected_rows(Ref, multistep_step1, Query1),
    [Date] = row_values(FirstAiredRow),
    FromDate = list_to_integer(Date),

    ToDate = FromDate + 15,

    Query2 = io_lib:format(
        "SELECT CAST(season_id AS Utf8) AS season_id, CAST(episode_id AS Utf8) AS episode_id, title, CAST(air_date AS Utf8) AS air_date FROM episodes "
        "WHERE series_id = ~p AND air_date >= ~p AND air_date <= ~p;",
        [SeriesId, FromDate, ToDate]
    ),

    Rows = selected_rows(Ref, multistep_step2, Query2),
    lists:foreach(fun(Row) ->
        [SId, EId, Title, AirDate] = row_values(Row),
        io:format("Episode: S~pE~p ~p (aired: ~p)~n", [SId, EId, Title, AirDate])
    end, Rows).

select_seasons_by_series(Ref) ->
    SeriesList = [1, 2],
    InClause = string:join([integer_to_list(X) || X <- SeriesList], ", "),

    Query = io_lib:format(
        "SELECT CAST(series_id AS Utf8) AS series_id, CAST(season_id AS Utf8) AS season_id, title, CAST(first_aired AS Date) AS first_aired "
        "FROM seasons WHERE series_id IN (~s) ORDER BY season_id;",
        [InClause]
    ),

    Rows = selected_rows(Ref, select_seasons_by_series, Query),
    lists:foreach(fun(Row) ->
        [SeriesId, SeasonId, Title, FirstAired] = row_values(Row),
        io:format("Season: Series=~p, Season=~p, Title=~p, FirstAired=~p~n",
                 [SeriesId, SeasonId, Title, FirstAired])
    end, Rows).

drop_tables(Ref) ->
    Tables = ["series", "seasons", "episodes"],

    lists:foreach(fun(Table) ->
        Query = io_lib:format("DROP TABLE ~s;", [Table]),
        case odbc:sql_query(Ref, lists:flatten(Query)) of
            {updated, _} -> ok;
            {error, _} -> ok
        end
    end, Tables).

escape_string(String) ->
    EscapedBackslash = string:replace(String, "\\", "\\\\", all),
    lists:flatten(string:replace(EscapedBackslash, "\"", "\\\"", all)).

execute_update(Ref, Query) ->
    case odbc:sql_query(Ref, lists:flatten(Query)) of
        {updated, _} -> ok;
        Error -> throw({query_failed, update, Error})
    end.

selected_rows(Ref, Step, Query) ->
    case odbc:sql_query(Ref, lists:flatten(Query)) of
        {selected, _, Rows} -> Rows;
        Error -> throw({query_failed, Step, Error})
    end.

selected_param_rows(Ref, Step, Query, Params) ->
    case odbc:param_query(Ref, lists:flatten(Query), Params) of
        {selected, _, Rows} -> Rows;
        Error -> throw({query_failed, Step, Error})
    end.

row_values(Row) when is_tuple(Row) ->
    tuple_to_list(Row);
row_values(Row) ->
    Row.
