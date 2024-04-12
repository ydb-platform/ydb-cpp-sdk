add_ydb_test(NAME operation_id-ut
<<<<<<< HEAD
=======
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/src/library/operation_id
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  SOURCES
  operation_id_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  library-operation_id
  cpp-testing-unittest
)
