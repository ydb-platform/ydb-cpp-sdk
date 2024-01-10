LIBRARY()

SRCS(
    helpers.cpp
)

PEERDIR(
    client/iam
    client/ydb_types/credentials
    ydb/library/yql/public/issue/protos
)

END()
