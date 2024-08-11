add_ydb_test(NAME yt-string-enum_ut GTEST
  SOURCES
    enum_ut.cpp
  LINK_LIBRARIES
    yt-string
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-string-format_ut GTEST
  SOURCES
    format_ut.cpp
  LINK_LIBRARIES
    yt-string
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-string-guid_ut GTEST
  SOURCES
    guid_ut.cpp
  LINK_LIBRARIES
    yt-string
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-string-string_ut GTEST
  SOURCES
    string_ut.cpp
  LINK_LIBRARIES
    yt-string
    GTest::gtest_main
  LABELS
    unit
)
