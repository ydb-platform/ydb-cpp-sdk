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
  gtest.cpp
  checks.cpp
  junit.cpp
  plugin.cpp
  registar.cpp
  tests_data.cpp
  utmain.cpp
)
