LIBRARY()

NO_UTIL()
ALLOCATOR_IMPL()

PEERDIR(
    library/cpp/malloc/api
    contrib/libs/mimalloc
)

SRCS(
    info.cpp
)

END()

RECURSE(
    link_test
)
