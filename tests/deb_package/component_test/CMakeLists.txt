cmake_minimum_required(VERSION 3.10)
project(test_ydb_cpp_sdk_components LANGUAGES CXX)

set(YDB_TEST_PACKAGE_NAME "ydb-cpp-sdk" CACHE STRING "Package name spelling to test")

if (YDB_TEST_COMPONENTS)
  find_package(
    ${YDB_TEST_PACKAGE_NAME} REQUIRED CONFIG
    COMPONENTS ${YDB_TEST_COMPONENTS}
  )
else()
  find_package(${YDB_TEST_PACKAGE_NAME} REQUIRED CONFIG)
endif()

foreach(component IN LISTS YDB_TEST_COMPONENTS)
  if (NOT TARGET YDB-CPP-SDK::${component})
    message(FATAL_ERROR "YDB SDK component target is missing: ${component}")
  endif()
endforeach()

if (NOT TARGET YDB-CPP-SDK::libydb-cpp)
  message(FATAL_ERROR "YDB SDK core target is missing")
endif()
