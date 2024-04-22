UNITTEST_FOR(util)

SUBSCRIBER(g:util-subscribers)

SRCS(
    folder/dirut_ut.cpp
    folder/filelist_ut.cpp
    folder/fts_ut.cpp
    folder/iterator_ut.cpp
    folder/path_ut.cpp
    folder/pathsplit_ut.cpp
)

PEERDIR(
    src/library/threading/future
)

INCLUDE(${ARCADIA_ROOT}/src/util/tests/ya_util_tests.inc)

END()
