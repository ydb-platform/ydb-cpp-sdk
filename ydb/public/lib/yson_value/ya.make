LIBRARY()

SRCS(
    ydb_yson_value.cpp
)

PEERDIR(
    library/cpp/yson
    library/cpp/yson/node
    client/ydb_result
    client/ydb_value
    ydb/library/uuid
)

END()
