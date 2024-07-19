add_ydb_test(NAME library-digest-murmur_ut
  SOURCES
    ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/digest/murmur
  LINK_LIBRARIES
    digest-murmur
    cpp-testing-unittest_main
)
