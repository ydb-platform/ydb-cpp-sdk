UNITTEST_FOR(client/ydb_topic)

IF (SANITIZER_TYPE == "thread" OR WITH_VALGRIND)
    TIMEOUT(1200)
    SIZE(LARGE)
    TAG(ya:fat)
ELSE()
    TIMEOUT(600)
    SIZE(MEDIUM)
ENDIF()

FORK_SUBTESTS()

PEERDIR(
    library/cpp/testing/gmock_in_unittest
    ydb/core/testlib/default
    ydb/public/lib/json_value
    ydb/public/lib/yson_value
    client/ydb_driver
    client/ydb_persqueue_core
    client/ydb_persqueue_core/impl
    client/ydb_persqueue_core/ut/ut_utils
    client/ydb_topic/codecs

    client/ydb_topic
    client/ydb_topic/impl
    client/ydb_topic/ut/ut_utils
)

YQL_LAST_ABI_VERSION()

SRCS(
    basic_usage_ut.cpp
    describe_topic_ut.cpp
    local_partition_ut.cpp
    topic_to_table_ut.cpp
)

END()
