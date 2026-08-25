cmake_minimum_required(VERSION 3.10)
project(test_ydb_cpp_sdk)

set(CMAKE_CXX_STANDARD 20)

option(YDB_TEST_GRPC_PKGCONFIG_FALLBACK
  "Disable gRPC config discovery and require the SDK pkg-config fallback"
  OFF
)
if (YDB_TEST_GRPC_PKGCONFIG_FALLBACK)
  set(CMAKE_DISABLE_FIND_PACKAGE_gRPC TRUE)
endif()

find_package(
    ydb-cpp-sdk REQUIRED CONFIG
    COMPONENTS Driver Iam OpenTelemetryMetrics OpenTelemetryTrace
)

foreach(target IN ITEMS gRPC::grpc++ absl::flat_hash_map)
  if (NOT TARGET ${target})
    message(FATAL_ERROR "Required dependency target is missing: ${target}")
  endif()
endforeach()

add_executable(test_app main.cpp)

target_link_libraries(test_app
    PRIVATE
    YDB-CPP-SDK::Driver
    YDB-CPP-SDK::Iam
    YDB-CPP-SDK::OpenTelemetryMetrics
    YDB-CPP-SDK::OpenTelemetryTrace
)
