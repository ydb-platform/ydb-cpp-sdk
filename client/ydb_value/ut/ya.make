UNITTEST_FOR(client/ydb_value)

IF (SANITIZER_TYPE == "thread")
    TIMEOUT(1200)
    SIZE(LARGE)
    TAG(ya:fat)
ELSE()
    TIMEOUT(600)
    SIZE(MEDIUM)
ENDIF()

FORK_SUBTESTS()

PEERDIR(
    ydb/public/lib/json_value
    ydb/public/lib/yson_value
    client/ydb_params
)

SRCS(
    value_ut.cpp
)

END()
