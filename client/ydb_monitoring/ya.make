LIBRARY()

SRCS(
    monitoring.cpp
)

GENERATE_ENUM_SERIALIZATION(monitoring.h)

PEERDIR(
    client/ydb_proto
    client/impl/ydb_internal/make_request
    client/ydb_common_client/impl
    client/ydb_driver
)

END()

