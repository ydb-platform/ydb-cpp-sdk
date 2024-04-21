PY23_TEST()

SUBSCRIBER(g:util-subscribers)

SRCDIR(src/util/memory)

PY_SRCS(
    NAMESPACE util.memory
    blob_ut.pyx
)

TEST_SRCS(
    test_memory.py
)

END()
