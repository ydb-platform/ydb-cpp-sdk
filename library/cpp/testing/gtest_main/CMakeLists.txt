add_library(cpp-testing-gtest_main)
target_link_libraries(cpp-testing-gtest_main PUBLIC
  cpp-testing-gtest
  GTest::gtest_main
)
target_sources(cpp-testing-gtest_main PRIVATE
  main.cpp
)

