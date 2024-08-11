add_ydb_test(NAME yt-misc-enum_ut GTEST
  SOURCES
    enum_ut.cpp
  LINK_LIBRARIES
    yt-misc
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-misc-guid_ut GTEST
  SOURCES
    guid_ut.cpp
  LINK_LIBRARIES
    yt-misc
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-misc-preprocessor_ut GTEST
  SOURCES
    preprocessor_ut.cpp
  LINK_LIBRARIES
    yt-misc
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME yt-misc-strong_typedef_ut GTEST
  SOURCES
    strong_typedef_ut.cpp
  LINK_LIBRARIES
    yt-misc
    GTest::gtest_main
  LABELS
    unit
)
