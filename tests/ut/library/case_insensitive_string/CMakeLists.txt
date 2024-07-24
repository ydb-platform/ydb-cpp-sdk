add_ydb_test(NAME library-case_insensitive_string_ut
  SOURCES
    case_insensitive_string_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string
  LINK_LIBRARIES
    library-case_insensitive_string
    cpp-testing-unittest_main
  LABELS
    unit
)
