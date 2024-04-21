GTEST(unittester-small-containers)

INCLUDE(${ARCADIA_ROOT}/src/library/yt/ya_cpp.make.inc)

SRCS(
    compact_flat_map_ut.cpp
    compact_heap_ut.cpp
    compact_set_ut.cpp
    compact_vector_ut.cpp
    compact_queue_ut.cpp
)

PEERDIR(
    src/library/yt/small_containers
    src/library/testing/gtest
)

END()
