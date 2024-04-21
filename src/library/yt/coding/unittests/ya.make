GTEST()

INCLUDE(${ARCADIA_ROOT}/src/library/yt/ya_cpp.make.inc)

SRCS(
    zig_zag_ut.cpp
    varint_ut.cpp
)

PEERDIR(
    src/library/yt/coding
    src/library/testing/gtest
)

END()
