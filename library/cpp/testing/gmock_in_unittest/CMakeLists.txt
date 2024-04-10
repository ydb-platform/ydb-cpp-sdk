add_library(cpp-testing-gmock_in_unittest)
target_link_libraries(cpp-testing-gmock_in_unittest PUBLIC
  yutil
  GTest::gmock
  GTest::gtest
  cpp-testing-gtest_extensions
  cpp-testing-unittest
)
target_sources(cpp-testing-gmock_in_unittest PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/gmock_in_unittest/events.cpp
)

add_global_library_for(cpp-testing-gmock_in_unittest.global cpp-testing-gmock_in_unittest)
target_link_libraries(cpp-testing-gmock_in_unittest.global PUBLIC
  yutil
  GTest::gmock
  GTest::gtest
  cpp-testing-gtest_extensions
  cpp-testing-unittest
)
target_sources(cpp-testing-gmock_in_unittest.global PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/gmock_in_unittest/registration.cpp
)

# Unit tests
add_subdirectory(example_ut)
