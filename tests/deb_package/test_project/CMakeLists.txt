cmake_minimum_required(VERSION 3.10)
project(test_ydb_cpp_sdk)

set(CMAKE_CXX_STANDARD 20)

find_package(ydb-cpp-sdk REQUIRED)

add_executable(test_app main.cpp)

target_link_libraries(test_app
    PRIVATE
    YDB-CPP-SDK::Driver
    YDB-CPP-SDK::Iam
    YDB-CPP-SDK::OpenTelemetryMetrics
    YDB-CPP-SDK::OpenTelemetryTrace
)
