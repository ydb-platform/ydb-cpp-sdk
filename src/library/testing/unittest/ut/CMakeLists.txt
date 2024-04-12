add_executable(cpp-testing-unittest_ut)
target_link_libraries(cpp-testing-unittest_ut PRIVATE
  cpp-testing-unittest_main
)
target_sources(cpp-testing-unittest_ut PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

set_target_properties(cpp-testing-unittest_ut
  PROPERTIES SPLIT_FACTOR 1
)
add_yunittest(
  NAME cpp-testing-unittest
  TEST_TARGET cpp-testing-unittest_ut
)
