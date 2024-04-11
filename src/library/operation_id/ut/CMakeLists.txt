add_ydb_test(NAME operation_id-ut
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/operation_id
  SOURCES
  ../operation_id_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  library-operation_id
  cpp-testing-unittest
)
