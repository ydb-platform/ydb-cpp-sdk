LIBRARY()

SRCS(
    value.cpp
)

GENERATE_ENUM_SERIALIZATION(value.h)

PEERDIR(
    library/cpp/containers/stack_vector
    ydb/public/api/protos
    client/impl/ydb_internal/value_helpers
    client/ydb_types/fatal_error_handlers
    ydb/library/yql/public/decimal
    ydb/library/uuid
)

END()

RECURSE_FOR_TESTS(
    ut
)
