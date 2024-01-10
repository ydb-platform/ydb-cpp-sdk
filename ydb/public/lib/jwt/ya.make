LIBRARY()

SRCS(
    jwt.cpp
    jwt.h
)

PEERDIR(
    contrib/libs/jwt-cpp
    library/cpp/json
    client/impl/ydb_internal/common
)

END()
