add_library(cpp-testing-unittest)
target_link_libraries(cpp-testing-unittest PUBLIC
  yutil
  library-cpp-colorizer
  library-cpp-dbg_output
  library-cpp-diff
  library-cpp-json
  cpp-json-writer
  cpp-testing-common
  cpp-testing-hook
)
target_sources(cpp-testing-unittest PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/unittest/gtest.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/unittest/checks.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/unittest/junit.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/unittest/plugin.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/unittest/registar.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/unittest/tests_data.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/testing/unittest/utmain.cpp
)

# Unit tests
add_subdirectory(fat)
add_subdirectory(ut)
