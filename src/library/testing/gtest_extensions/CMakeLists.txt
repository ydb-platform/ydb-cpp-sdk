add_library(cpp-testing-gtest_extensions)
target_link_libraries(cpp-testing-gtest_extensions PUBLIC
  yutil
  GTest::gmock
  GTest::gtest
)
target_sources(cpp-testing-gtest_extensions PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gtest_extensions/assertions.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gtest_extensions/gtest_extensions.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gtest_extensions/matchers.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gtest_extensions/pretty_printers.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/gtest_extensions/probe.cpp
)

# Unit tests
add_subdirectory(ut)
