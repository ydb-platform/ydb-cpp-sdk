cmake_minimum_required(VERSION 3.14)
project(yandex-opentelemetry-cpp VERSION 1.12.0)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(WITH_OTLP_HTTP OFF CACHE BOOL "" FORCE)
set(WITH_OTLP_GRPC OFF CACHE BOOL "" FORCE)
set(WITH_PROMETHEUS OFF CACHE BOOL "" FORCE)
set(WITH_EXAMPLES OFF CACHE BOOL "" FORCE)
set(OTELCPP_MAINTAINER_MODE OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)
set(WITH_STL "CXX20" CACHE STRING "" FORCE)
set(OPENTELEMETRY_INSTALL ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  opentelemetry-cpp
  GIT_REPOSITORY https://github.com/open-telemetry/opentelemetry-cpp.git
  GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(opentelemetry-cpp)
