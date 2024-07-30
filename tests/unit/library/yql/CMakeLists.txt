add_ydb_test(NAME yql-utils_ut
  SOURCES
  utf8_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-utils
  LABELS
  unit
)

add_ydb_test(NAME yql-public-issue_ut
  SOURCES
  yql_issue_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-public-issue
<<<<<<< HEAD:tests/unit/library/yql/CMakeLists.txt
  unicode_normalization
  LABELS
  unit
=======
  unicode-normalization
>>>>>>> 740dcae8d (Returned library and switched client to ol util and library):tests/library/yql/CMakeLists.txt
)

add_ydb_test(NAME yql-public-decimal_ut
  SOURCES
  yql_decimal_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-public-decimal
  LABELS
  unit
)

add_ydb_test(NAME yql-public-wide_int_ut
  SOURCES
  yql_wide_int_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  yql-public-decimal
  LABELS
  unit
)
