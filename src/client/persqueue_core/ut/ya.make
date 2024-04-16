UNITTEST_FOR(client/ydb_persqueue_core)

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
    src/library/testing/gmock_in_unittest
    ydb/core/testlib/default
    ydb/public/lib/json_value
    ydb/public/lib/yson_value
    client/ydb_driver
    client/ydb_persqueue_core
    client/ydb_persqueue_core/impl
    client/ydb_persqueue_core/ut/ut_utils
    client/ydb_persqueue_public/codecs
)

YQL_LAST_ABI_VERSION()

SRCS(
    common_ut.cpp
    read_session_ut.cpp
    basic_usage_ut.cpp
    compress_executor_ut.cpp
    compression_ut.cpp
    retry_policy_ut.cpp
)

END()

RECURSE_FOR_TESTS(
    with_offset_ranges_mode_ut
)
