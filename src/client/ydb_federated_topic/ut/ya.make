UNITTEST_FOR(client/ydb_federated_topic)

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

    client/ydb_federated_topic
    client/ydb_federated_topic/impl
)

YQL_LAST_ABI_VERSION()

SRCS(
    basic_usage_ut.cpp
)

END()
