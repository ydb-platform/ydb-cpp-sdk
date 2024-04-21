GTEST(unittester-library-string)

INCLUDE(${ARCADIA_ROOT}/src/library/yt/ya_cpp.make.inc)

SRCS(
    enum_ut.cpp
    format_ut.cpp
    guid_ut.cpp
    string_ut.cpp
)

PEERDIR(
    src/library/yt/string
    src/library/testing/gtest
)

END()
