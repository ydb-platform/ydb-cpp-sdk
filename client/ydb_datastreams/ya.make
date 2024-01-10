LIBRARY()

SRCS(
    datastreams.cpp
)

PEERDIR(
    ydb/library/grpc/client
    library/cpp/string_utils/url
    ydb/public/api/grpc/draft
    ydb/public/lib/operation_id
    client/impl/ydb_internal/make_request
    client/ydb_driver
)

END()
