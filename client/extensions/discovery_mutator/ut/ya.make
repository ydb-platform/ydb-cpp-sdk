UNITTEST_FOR(client/extensions/discovery_mutator)

IF (SANITIZER_TYPE == "thread")
    TIMEOUT(1200)
    SIZE(LARGE)
    TAG(ya:fat)
ELSE()
    TIMEOUT(600)
    SIZE(MEDIUM)
ENDIF()

FORK_SUBTESTS()

PEERDIR(
    client/ydb_table
)

SRCS(
    discovery_mutator_ut.cpp
)

END()
