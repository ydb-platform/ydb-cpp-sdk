LIBRARY()

SRCS(
    iam.cpp
)

PEERDIR(
    ydb/public/api/client/yc_public/iam
    client/iam/impl
    client/iam/common
)

END()
