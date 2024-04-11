add_ydb_test(NAME yql-public-decimal_ut
  INCLUDE_DIRS
  ${YDB_SDK_SOURCE_DIR}/src/library/yql/public/decimal
  SOURCES
  ${YDB_SDK_SOURCE_DIR}/src/library/yql/public/decimal/ut/yql_decimal_ut.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yql/public/decimal/ut/yql_wide_int_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-public-decimal
)
