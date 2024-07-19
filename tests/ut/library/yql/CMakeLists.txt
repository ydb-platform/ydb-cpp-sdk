add_ydb_test(NAME yql-utils_ut
  SOURCES
  utf8_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-utils
)

add_ydb_test(NAME yql-public-issue_ut
  SOURCES
  yql_issue_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-public-issue
  unicode_normalization
)

add_ydb_test(NAME yql-public-decimal_ut
  SOURCES
  yql_decimal_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-public-decimal
)

add_ydb_test(NAME yql-public-wide_int_ut
  SOURCES
  yql_wide_int_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-public-decimal
)
