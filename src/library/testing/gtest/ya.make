LIBRARY()

PROVIDES(test_framework)

SRCS(
    gtest.cpp
    main.cpp
    matchers.cpp
)

PEERDIR(
    contrib/restricted/googletest/googlemock
    contrib/restricted/googletest/googletest
    src/library/string_utils/relaxed_escaper
    src/library/testing/common
    src/library/testing/gtest_extensions
    src/library/testing/hook
)

END()

RECURSE_FOR_TESTS(ut)
