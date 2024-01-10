LIBRARY()

SRCS(
    rate_limiter.cpp
)

PEERDIR(
    ydb/public/api/grpc
    client/ydb_common_client/impl
    client/ydb_driver
)

END()
