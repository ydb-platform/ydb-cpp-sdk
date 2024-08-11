add_ydb_test(NAME yt-small_containers-compact_flat_map_ut GTEST
  SOURCES
    compact_flat_map_ut.cpp
  LINK_LIBRARIES
    yt-small_containers
    yt-string
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-small_containers-compact_heap_ut GTEST
  SOURCES
    compact_heap_ut.cpp
  LINK_LIBRARIES
    yt-small_containers
    yt-string
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-small_containers-compact_queue_ut GTEST
  SOURCES
    compact_queue_ut.cpp
  LINK_LIBRARIES
    yt-small_containers
    yt-string
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-small_containers-compact_set_ut GTEST
  SOURCES
    compact_set_ut.cpp
  LINK_LIBRARIES
    yt-small_containers
    yt-string
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-small_containers-compact_vector_ut GTEST
  SOURCES
    compact_vector_ut.cpp
  LINK_LIBRARIES
    yt-small_containers
    yt-string
    GTest::gtest_main
    GTest::gmock
  LABELS
    unit
)
