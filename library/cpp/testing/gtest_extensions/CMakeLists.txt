add_library(cpp-testing-gtest_extensions)
target_link_libraries(cpp-testing-gtest_extensions PUBLIC
  yutil
  GTest::gmock
  GTest::gtest
)
target_sources(cpp-testing-gtest_extensions PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/gtest_extensions/assertions.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/gtest_extensions/gtest_extensions.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/gtest_extensions/matchers.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/gtest_extensions/pretty_printers.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/gtest_extensions/probe.cpp
)

# Unit tests
add_subdirectory(ut)
