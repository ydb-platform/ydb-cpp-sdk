add_executable(cpp-testing-gmock_in_unittest_ut)
target_link_libraries(cpp-testing-gmock_in_unittest_ut PRIVATE
  cpp-testing-gmock_in_unittest
  cpp-testing-unittest_main
)
target_sources(cpp-testing-gmock_in_unittest_ut PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/example_ut.cpp
)

set_target_properties(cpp-testing-gmock_in_unittest_ut
  PROPERTIES SPLIT_FACTOR 1
)
add_yunittest(
  NAME cpp-testing-gmock_in_unittest
  TEST_TARGET cpp-testing-gmock_in_unittest_ut
)
