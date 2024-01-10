LIBRARY()

SRCS(
    table.cpp
    proto_accessor.cpp
)

GENERATE_ENUM_SERIALIZATION(table_enum.h)

PEERDIR(
    ydb/public/api/protos
    client/impl/ydb_internal/make_request
    client/impl/ydb_internal/kqp_session_common
    client/impl/ydb_internal/retry
    client/ydb_driver
    client/ydb_params
    client/ydb_proto
    client/ydb_result
    client/ydb_scheme
    client/ydb_table/impl
    client/ydb_table/query_stats
    client/ydb_types/operation
    client/ydb_value
)

END()
