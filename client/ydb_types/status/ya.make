LIBRARY()

SRCS(
    status.cpp
)

PEERDIR(
    library/cpp/threading/future
    client/impl/ydb_internal/plain_status
    client/ydb_types
    client/ydb_types/fatal_error_handlers
    ydb/library/yql/public/issue
)

END()
