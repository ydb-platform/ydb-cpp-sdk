UNITTEST_FOR(src/library/http/simple)

PEERDIR(
    src/library/http/misc
    src/library/testing/mock_server
)

SRCS(
    http_ut.cpp
    https_ut.cpp
)

DEPENDS(src/library/http/simple/ut/https_server)

DATA(arcadia/src/library/http/simple/ut/https_server)

END()

RECURSE(
    https_server
)
