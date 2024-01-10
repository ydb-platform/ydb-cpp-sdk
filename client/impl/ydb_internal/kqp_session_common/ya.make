LIBRARY()

SRCS(
    kqp_session_common.cpp
)

PEERDIR(
    library/cpp/threading/future
    ydb/public/lib/operation_id/protos
    client/impl/ydb_endpoints
)


END()
