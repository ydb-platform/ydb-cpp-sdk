GTEST(unittester-library-misc)

INCLUDE(${ARCADIA_ROOT}/src/library/yt/ya_cpp.make.inc)

SRCS(
    enum_ut.cpp
    guid_ut.cpp
    preprocessor_ut.cpp
)

PEERDIR(
    src/library/yt/misc
)

END()
