-module(ydb_report).
-export([main/0]).

cases(Module, Entries) ->
    lists:flatmap(fun({group, Name}) ->
                         {Name, _, Children} = lists:keyfind(Name, 1, Module:groups()),
                         cases(Module, Children);
                    (Case) when is_atom(Case) -> [Case]
                 end, Entries).

main() ->
    Suites = [odbc_start_SUITE, odbc_connect_SUITE, odbc_query_SUITE, odbc_data_type_SUITE],
    case os:getenv("ODBC_DISCOVER_ONLY") of
        "1" -> lists:foreach(fun(Suite) ->
                     lists:foreach(fun(Case) -> io:format("@YDB_INVENTORY\t~s.~s~n", [Suite, Case]) end,
                                   cases(Suite, Suite:all())) end, Suites);
        _ -> Native = os:getenv("ODBC_NATIVE_RESULTS_DIR"),
             Logs = filename:join(Native, "ct"),
             file:make_dir(Logs),
             ct:run_test([{dir, os:getenv("YDB_ERLANG_TEST_DIR")}, {suite, Suites},
                          {auto_compile, false}, {logdir, Logs},
                          {ct_hooks, [{cth_surefire, [{path, filename:join(Native,"ct.xml")}]}]}])
    end,
    halt().
