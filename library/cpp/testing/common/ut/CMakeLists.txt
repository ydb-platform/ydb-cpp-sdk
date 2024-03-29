add_executable(cpp-testing-common_ut)
target_link_libraries(cpp-testing-common_ut PRIVATE
  cpp-testing-gtest_main
)
target_sources(cpp-testing-common_ut PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/env_ut.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/network_ut.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/scope_ut.cpp
)

set_target_properties(cpp-testing-common_ut
  PROPERTIES SPLIT_FACTOR 1
)
add_yunittest(
  NAME cpp-testing-common
  TEST_TARGET cpp-testing-common_ut
)