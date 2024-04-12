PY23_TEST()

SUBSCRIBER(g:util-subscribers)

SRCDIR(src/util/string)

PY_SRCS(
    NAMESPACE util.string
    cast_ut.pyx
)

TEST_SRCS(
    test_string.py
)

END()
