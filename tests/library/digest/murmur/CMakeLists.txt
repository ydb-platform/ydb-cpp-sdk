add_ydb_test(NAME library-digest-murmur-ut
  SOURCES
    ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/digest/murmur
  LINK_LIBRARIES
    cpp-digest-murmur
    cpp-testing-unittest_main
)
