LIBRARY()

SRCS(
    scheme.cpp
)

GENERATE_ENUM_SERIALIZATION(scheme.h)

PEERDIR(
    client/impl/ydb_internal/make_request
    client/ydb_common_client/impl
    client/ydb_driver
)

END()
