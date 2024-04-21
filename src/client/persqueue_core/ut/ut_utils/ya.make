LIBRARY()

SRCS(
    data_plane_helpers.cpp
    sdk_test_setup.h
    test_utils.h
    test_server.h
    test_server.cpp
    ut_utils.h
    ut_utils.cpp
)

PEERDIR(
    src/library/grpc/server
    src/library/testing/unittest
    src/library/threading/chunk_queue
    ydb/core/testlib/default
    src/library/persqueue/topic_parser_public
    client/ydb_driver
    client/ydb_persqueue_core
    client/ydb_persqueue_public
    client/ydb_table
)

YQL_LAST_ABI_VERSION()

END()
