UNITTEST_FOR(ydb/public/lib/json_value)

TIMEOUT(600)

SIZE(MEDIUM)

FORK_SUBTESTS()

SRCS(
    ydb_json_value_ut.cpp
)

PEERDIR(
    library/cpp/json
    library/cpp/testing/unittest
    client/ydb_proto
    client/ydb_params
)

END()
