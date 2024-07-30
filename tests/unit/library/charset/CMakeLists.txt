add_ydb_test(NAME charset-ci_string_ut
  SOURCES
    ci_string_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/charset
  LINK_LIBRARIES
    charset-lite
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME charset-codepage_ut
  SOURCES
    codepage_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/charset
  LINK_LIBRARIES
    charset
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME charset-iconv_ut
  SOURCES
    iconv_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/charset
  LINK_LIBRARIES
    charset
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME charset-recyr_int_ut
  SOURCES
    recyr_int_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/charset
  LINK_LIBRARIES
    charset
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME charset-wide_ut
  SOURCES
    wide_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/charset
  LINK_LIBRARIES
    charset
    cpp-testing-unittest_main
  LABELS
    unit
)
