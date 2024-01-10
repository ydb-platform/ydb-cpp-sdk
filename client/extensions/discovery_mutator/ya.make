LIBRARY()

SRCS(
    discovery_mutator.cpp
)

PEERDIR(
    client/ydb_extension
)

END()

RECURSE_FOR_TESTS(
    ut
)
