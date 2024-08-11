add_ydb_test(NAME iterator-filtering_ut GTEST
  SOURCES
    filtering_ut.cpp
  LINK_LIBRARIES
    iterator
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME iterator-functools_ut GTEST
  SOURCES
    functools_ut.cpp
  LINK_LIBRARIES
    iterator
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME iterator-iterate_keys_ut GTEST
  SOURCES
    iterate_keys_ut.cpp
  LINK_LIBRARIES
    iterator
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME iterator-iterate_values_ut GTEST
  SOURCES
    iterate_values_ut.cpp
  LINK_LIBRARIES
    iterator
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME iterator-mapped_ut GTEST
  SOURCES
    mapped_ut.cpp
  LINK_LIBRARIES
    iterator
    GTest::gtest_main
  LABELS
    unit
)

add_ydb_test(NAME iterator-zip_ut GTEST
  SOURCES
    zip_ut.cpp
  LINK_LIBRARIES
    iterator
    GTest::gtest_main
  LABELS
    unit
)
