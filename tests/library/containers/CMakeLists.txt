add_ydb_test(NAME library-containers-disjoint_interval_tree_ut
  SOURCES
    disjoint_interval_tree_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/containers/disjoint_interval_tree
  LINK_LIBRARIES
    containers-disjoint_interval_tree
    cpp-testing-unittest_main
)

add_ydb_test(NAME library-containers-intrusive_rb_tree_ut
  SOURCES
    intrusive_rb_tree_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/containers/intrusive_rb_tree
  LINK_LIBRARIES
    containers-intrusive_rb_tree
    cpp-testing-unittest_main
)

add_ydb_test(NAME library-containers-paged_vector_ut
  SOURCES
    paged_vector_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/containers/paged_vector
  LINK_LIBRARIES
    containers-paged_vector
    cpp-testing-unittest_main
)

add_ydb_test(NAME library-containers-stack_vector_ut
  SOURCES
    stack_vector_ut.cpp
  LINK_LIBRARIES
    containers-stack_vector
    cpp-testing-unittest_main
)
