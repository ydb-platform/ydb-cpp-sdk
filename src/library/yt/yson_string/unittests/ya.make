GTEST()

INCLUDE(${ARCADIA_ROOT}/src/library/yt/ya_cpp.make.inc)

SRCS(
    convert_ut.cpp
    saveload_ut.cpp
)

PEERDIR(
    src/library/yt/yson_string
    src/library/testing/gtest
    src/library/testing/gtest_extensions
)

END()
