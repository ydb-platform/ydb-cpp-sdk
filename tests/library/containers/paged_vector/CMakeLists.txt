add_ydb_test(NAME library-containers-paged_vector-ut
  SOURCES
    ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/containers/paged_vector
  LINK_LIBRARIES
    containers-paged_vector
    cpp-testing-unittest_main
)
