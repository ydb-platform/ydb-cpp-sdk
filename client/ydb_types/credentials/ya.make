LIBRARY()

SRCS(
    credentials.cpp
)

PEERDIR(
    ydb/library/login
    ydb/public/api/grpc
    client/ydb_types/status
    ydb/library/yql/public/issue
)

END()
