add_executable(cpp-testing-gtest_ut)
target_link_libraries(cpp-testing-gtest_ut PRIVATE
  cpp-testing-gtest_main
)
target_sources(cpp-testing-gtest_ut PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/matchers_ut.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ut.cpp
)

set_target_properties(cpp-testing-gtest_ut
  PROPERTIES SPLIT_FACTOR 1
)
add_yunittest(
  NAME cpp-testing-gtest
  TEST_TARGET cpp-testing-gtest_ut
)