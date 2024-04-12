LIBRARY()

PROVIDES(test_framework)

PEERDIR(
    src/library/colorizer
    src/library/dbg_output
    src/library/diff
    src/library/json
    src/library/json/writer
    src/library/testing/common
    src/library/testing/hook
)

SRCS(
    gtest.cpp
    checks.cpp
    junit.cpp
    plugin.cpp
    registar.cpp
    tests_data.cpp
    utmain.cpp
)

END()

RECURSE_FOR_TESTS(
    fat
    pytests
    ut
)
