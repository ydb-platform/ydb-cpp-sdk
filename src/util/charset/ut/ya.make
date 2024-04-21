UNITTEST_FOR(src/util/charset)

SUBSCRIBER(g:util-subscribers)

DATA(arcadia/src/util/charset/ut/utf8)

SRCS(
    utf8_ut.cpp
    wide_ut.cpp
)

INCLUDE(${ARCADIA_ROOT}/src/util/tests/ya_util_tests.inc)

REQUIREMENTS(ram:17)

END()
