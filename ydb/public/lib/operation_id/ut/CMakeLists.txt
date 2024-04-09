add_ydb_test(NAME operation_id-ut
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/ydb/public/lib/operation_id
  SOURCES
  ${CMAKE_SOURCE_DIR}/ydb/public/lib/operation_id/operation_id_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  public-lib-operation_id
  cpp-testing-unittest
)
