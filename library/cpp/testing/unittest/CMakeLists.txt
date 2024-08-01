if (YDB_SDK_TESTS)
  add_subdirectory(fat)
  add_subdirectory(ut)
endif()

add_library(cpp-testing-unittest)

target_link_libraries(cpp-testing-unittest PUBLIC
  yutil
  colorizer
  dbg_output
  diff
  json
  json-writer
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
