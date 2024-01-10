LIBRARY()

SRCS(
    persqueue.h
)

PEERDIR(
    client/ydb_persqueue_core
    client/ydb_persqueue_public/codecs
)

END()
