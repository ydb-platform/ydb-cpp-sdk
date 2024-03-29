add_executable(cpp-testing-unittest_fat)
target_link_libraries(cpp-testing-unittest_fat PRIVATE
  cpp-testing-unittest_main
)
target_sources(cpp-testing-unittest_fat PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/test_port_manager.cpp
)

set_target_properties(cpp-testing-unittest_fat
  PROPERTIES SPLIT_FACTOR 9
)
add_yunittest(
  NAME cpp-testing-unittest-fat
  TEST_TARGET cpp-testing-unittest_fat
)