cmake_minimum_required(VERSION 3.10)
project(test_ydb_cpp_sdk_components LANGUAGES CXX)

set(YDB_TEST_PACKAGE_NAME "ydb-cpp-sdk" CACHE STRING "Package name spelling to test")
option(YDB_TEST_PREDEFINED_RPC_TARGETS
  "Test an embedding project that already provides Protobuf and gRPC targets"
  OFF
)

if (YDB_TEST_PREDEFINED_RPC_TARGETS)
  add_library(protobuf::libprotobuf INTERFACE IMPORTED GLOBAL)
  add_library(gRPC::grpc++ INTERFACE IMPORTED GLOBAL)
  set(CMAKE_DISABLE_FIND_PACKAGE_Protobuf TRUE)
  set(CMAKE_DISABLE_FIND_PACKAGE_gRPC TRUE)
endif()

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

if (YDB_TEST_PREDEFINED_RPC_TARGETS)
  foreach(target IN ITEMS protobuf::libprotobuf gRPC::grpc++ absl::flat_hash_map)
    if (NOT TARGET ${target})
      message(FATAL_ERROR "Required dependency target is missing: ${target}")
    endif()
  endforeach()
endif()
