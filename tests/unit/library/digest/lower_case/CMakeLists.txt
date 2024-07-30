add_ydb_test(NAME library-digest-lower_case-hash_ops_ut
  SOURCES
    hash_ops_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/digest/lower_case
  LINK_LIBRARIES
    digest-lower_case
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME library-digest-lower_case-lchash_ut
  SOURCES
    lchash_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/digest/lower_case
  LINK_LIBRARIES
    digest-lower_case
    cpp-testing-unittest_main
  LABELS
    unit
)
