add_ydb_test(NAME library-containers-stack_vector-ut
  SOURCES
    ut.cpp
  LINK_LIBRARIES
    containers-stack_vector
    cpp-testing-unittest_main
)
