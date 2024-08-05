add_ydb_test(NAME containers-paged_vector_ut
  SOURCES
    paged_vector_ut.cpp
  LINK_LIBRARIES
    containers-paged_vector
    cpp-testing-unittest_main
  LABELS
    unit
)
