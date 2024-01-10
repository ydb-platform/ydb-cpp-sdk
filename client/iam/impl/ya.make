LIBRARY()

SRCS(
    iam_impl.h
)

PEERDIR(
    ydb/library/grpc/client
    library/cpp/http/simple
    library/cpp/json
    library/cpp/threading/atomic
    ydb/public/lib/jwt
    client/ydb_types/credentials
    client/iam/common
)

END()
