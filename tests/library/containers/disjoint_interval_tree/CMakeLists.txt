add_ydb_test(NAME library-containers-disjoint_interval_tree-ut
  SOURCES
    ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/containers/disjoint_interval_tree
  LINK_LIBRARIES
    containers-disjoint_interval_tree
    cpp-testing-unittest_main
)
