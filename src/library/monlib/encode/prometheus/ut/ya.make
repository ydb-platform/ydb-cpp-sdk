UNITTEST_FOR(src/library/monlib/encode/prometheus)

SRCS(
    prometheus_encoder_ut.cpp
    prometheus_decoder_ut.cpp
)

PEERDIR(
    src/library/monlib/encode/protobuf
)

END()
