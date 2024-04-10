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
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/unittest/gtest.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/unittest/checks.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/unittest/junit.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/unittest/plugin.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/unittest/registar.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/unittest/tests_data.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/testing/unittest/utmain.cpp
)

# Unit tests
add_subdirectory(fat)
add_subdirectory(ut)
