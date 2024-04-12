LIBRARY()

PEERDIR(
    contrib/restricted/googletest/googlemock
    contrib/restricted/googletest/googletest
    src/library/testing/gtest_extensions
    src/library/testing/unittest
)

SRCS(
    events.cpp
    GLOBAL registration.cpp
)

END()

RECURSE_FOR_TESTS(
    example_ut
)
