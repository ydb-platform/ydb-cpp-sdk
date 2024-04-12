GTEST()
SRCS(
    matchers_ut.cpp
    ut.cpp
)

DATA(
    arcadia/src/library/testing/gtest/ut/golden
)

PEERDIR(
    src/library/testing/hook
)

END()
