add_ydb_test(NAME yt-coding-varint_ut GTEST
  SOURCES
    varint_ut.cpp
  LINK_LIBRARIES
    yt-coding
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-coding-zig_zag_ut GTEST
  SOURCES
    zig_zag_ut.cpp
  LINK_LIBRARIES
    yt-coding
    GTest::gtest_main
  LABELS
    unit
)
