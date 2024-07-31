add_library(cpp-testing-gtest)
target_link_libraries(cpp-testing-gtest PUBLIC
  GTest::gmock
  GTest::gtest
  cpp-testing-common
  cpp-testing-gtest_extensions
  cpp-testing-hook
)
target_sources(cpp-testing-gtest PRIVATE
  gtest.cpp
  main.cpp
  matchers.cpp
)
