UNITTEST_FOR(src/library/http/io)

PEERDIR(
    src/library/http/server
)

SRCS(
    chunk_ut.cpp
    compression_ut.cpp
    headers_ut.cpp
    stream_ut.cpp
    stream_ut_medium.cpp
)

END()
