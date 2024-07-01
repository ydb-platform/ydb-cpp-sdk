add_ydb_test(NAME library-containers-intrusive_rb_tree-ut
  SOURCES
    ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/containers/intrusive_rb_tree
  LINK_LIBRARIES
    containers-intrusive_rb_tree
    cpp-testing-unittest_main
)
