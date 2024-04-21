PY23_TEST()

SUBSCRIBER(g:util-subscribers)

SRCDIR(src/util/system)

PY_SRCS(
    NAMESPACE util.system
    types_ut.pyx
)

TEST_SRCS(
    test_system.py
)

END()
