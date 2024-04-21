UNITTEST_FOR(src/library/monlib/dynamic_counters)

SRCS(
    contention_ut.cpp
    counters_ut.cpp
    encode_ut.cpp
)

PEERDIR(
    src/library/monlib/encode/protobuf
    src/library/monlib/encode/json
)

END()
