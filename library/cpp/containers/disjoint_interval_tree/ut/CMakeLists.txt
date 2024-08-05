add_ydb_test(NAME containers-disjoint_interval_tree_ut
  SOURCES
    disjoint_interval_tree_ut.cpp
  LINK_LIBRARIES
    containers-disjoint_interval_tree
    cpp-testing-unittest_main
  LABELS
    unit
)
