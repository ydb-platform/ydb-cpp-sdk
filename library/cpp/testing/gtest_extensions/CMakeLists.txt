if (YDB_SDK_TESTS)
  add_subdirectory(ut)
endif()

add_library(cpp-testing-gtest_extensions)

target_link_libraries(cpp-testing-gtest_extensions PUBLIC
  yutil
  GTest::gmock
  GTest::gtest
)

target_sources(cpp-testing-gtest_extensions PRIVATE
  assertions.cpp
  gtest_extensions.cpp
  matchers.cpp
  pretty_printers.cpp
  probe.cpp
)
