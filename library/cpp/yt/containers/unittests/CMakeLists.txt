add_ydb_test(NAME yt-containers-enum_indexed_array_ut GTEST
  SOURCES
    enum_indexed_array_ut.cpp
  LINK_LIBRARIES
    yt-containers
    yt-string
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-containers-sharded_set_ut GTEST
  SOURCES
    sharded_set_ut.cpp
  LINK_LIBRARIES
    yt-containers
    yt-string
    GTest::gtest_main
  LABELS
    unit
)
