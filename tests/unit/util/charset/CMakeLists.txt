add_ydb_test(NAME util-charset-utf8_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/charset
  SOURCES
    utf8_ut.cpp
  LINK_LIBRARIES
    util-charset
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME util-charset-wide_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/charset
  SOURCES
    wide_ut.cpp
  LINK_LIBRARIES
    util-charset
    cpp-testing-unittest_main
  LABELS
    unit
)
