LIBRARY()

SRCS(
    export.cpp
)

GENERATE_ENUM_SERIALIZATION(export.h)

PEERDIR(
    ydb/public/api/grpc
    ydb/public/api/protos
    client/ydb_common_client/impl
    client/ydb_driver
    client/ydb_proto
    client/ydb_types/operation
)

END()
