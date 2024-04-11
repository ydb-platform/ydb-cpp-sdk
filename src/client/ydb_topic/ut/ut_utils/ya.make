LIBRARY()

SRCS(
    managed_executor.cpp
    managed_executor.h
    topic_sdk_test_setup.cpp
    topic_sdk_test_setup.h
)

PEERDIR(
    ydb/library/grpc/server
    library/cpp/testing/unittest
    library/cpp/threading/chunk_queue
    ydb/core/testlib/default
    ydb/library/persqueue/topic_parser_public
    client/ydb_driver
    client/ydb_topic
    client/ydb_table
)

YQL_LAST_ABI_VERSION()

END()
