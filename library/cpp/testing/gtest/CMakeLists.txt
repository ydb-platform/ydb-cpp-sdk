# Providing two targets for the current googletest wrapper
# with and without 'main' entry point:
#   cpp-testing-gtest_main
#   cpp-testing-gtest
foreach(_suffix IN ITEMS "" "_main")
  add_library(cpp-testing-gtest${_suffix})
  target_link_libraries(cpp-testing-gtest${_suffix} PUBLIC
    GTest::gmock
    GTest::gtest${_suffix}
    cpp-testing-common
    cpp-testing-gtest_extensions
    cpp-testing-hook
  )
  target_sources(cpp-testing-gtest${_suffix} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/gtest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/matchers.cpp
  )
endforeach()

# Unit tests
add_subdirectory(ut)