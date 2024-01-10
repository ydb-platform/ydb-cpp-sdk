LIBRARY()

SRCS(
    stats.cpp
)

PEERDIR(
    contrib/libs/protobuf
    ydb/public/api/protos
    client/ydb_query
)

END()
