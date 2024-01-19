LIBRARY()

SRCS(
    grpc_client_low.cpp
)

PEERDIR(
    contrib/libs/grpc
    library/cpp/deprecated/atomic
)

END()
