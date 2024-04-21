UNITTEST_FOR(src/library/uri)

NO_OPTIMIZE()

NO_WSHADOW()

PEERDIR(
    src/library/html/entity
)

SRCS(
    location_ut.cpp
    uri-ru_ut.cpp
    uri_ut.cpp
)

END()
