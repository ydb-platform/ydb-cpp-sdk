LIBRARY()

SRCS(
    authenticator.cpp
    endpoint_pool.cpp
    state.cpp
)

PEERDIR(
    library/cpp/string_utils/quote
    library/cpp/threading/future
    client/impl/ydb_endpoints
    client/impl/ydb_internal/logger
    client/impl/ydb_internal/plain_status
    client/ydb_types/credentials
)

END()
