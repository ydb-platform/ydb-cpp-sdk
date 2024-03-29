add_executable(cpp-testing-gtest_extensions_ut)
target_link_libraries(cpp-testing-gtest_extensions_ut PRIVATE
  cpp-testing-gtest_main
)
target_sources(cpp-testing-gtest_extensions_ut PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/gtest_extensions_ut.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/probe_ut.cpp
)

set_target_properties(cpp-testing-gtest_extensions_ut
  PROPERTIES SPLIT_FACTOR 1
)
add_yunittest(
  NAME cpp-testing-gtest_extensions
  TEST_TARGET cpp-testing-gtest_extensions_ut
)