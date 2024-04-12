LIBRARY()

SRCS(
    env.cpp
    network.cpp
    probe.cpp
    scope.cpp
)

PEERDIR(
    src/library/json
)

END()

RECURSE_FOR_TESTS(ut)
