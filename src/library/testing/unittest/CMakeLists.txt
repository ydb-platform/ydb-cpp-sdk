add_library(cpp-testing-unittest)
<<<<<<< HEAD

=======
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
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
<<<<<<< HEAD

target_sources(cpp-testing-unittest PRIVATE
  gtest.cpp
  checks.cpp
  junit.cpp
  plugin.cpp
  registar.cpp
  tests_data.cpp
  utmain.cpp
)
=======
target_sources(cpp-testing-unittest PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest/gtest.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest/checks.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest/junit.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest/plugin.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest/registar.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest/tests_data.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest/utmain.cpp
)

# Unit tests
add_subdirectory(fat)
add_subdirectory(ut)
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
