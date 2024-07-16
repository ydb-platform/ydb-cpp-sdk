add_ydb_test(NAME library-charset-ci_string_ut
  SOURCES
    ci_string_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/charset
  LINK_LIBRARIES
    library-charset-lite
    cpp-testing-unittest_main
)

add_ydb_test(NAME library-charset-codepage_ut
  SOURCES
    codepage_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/charset
  LINK_LIBRARIES
    library-charset
    cpp-testing-unittest_main
)

add_ydb_test(NAME library-charset-iconv_ut
  SOURCES
    iconv_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/charset
  LINK_LIBRARIES
    library-charset
    cpp-testing-unittest_main
)

add_ydb_test(NAME library-charset-recyr_int_ut
  SOURCES
    recyr_int_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/charset
  LINK_LIBRARIES
    library-charset
    cpp-testing-unittest_main
)

add_ydb_test(NAME library-charset-wide_ut
  SOURCES
    wide_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/charset
  LINK_LIBRARIES
    library-charset
    cpp-testing-unittest_main
)
