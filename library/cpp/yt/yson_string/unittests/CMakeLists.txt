#[=[
TODO: missing `library/cpp/testing/gtest_extensions`
add_ydb_test(NAME yt-yson_string-convert_ut GTEST
  SOURCES
    convert_ut.cpp
  LINK_LIBRARIES
    yt-yson_string
    GTest::gtest_main
  LABELS
    unit
)
#]=]
add_ydb_test(NAME yt-yson_string-saveload_ut GTEST
  SOURCES
    saveload_ut.cpp
  LINK_LIBRARIES
    yt-yson_string
    GTest::gtest_main
  LABELS
    unit
)
