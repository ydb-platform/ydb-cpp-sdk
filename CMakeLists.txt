cmake_minimum_required(VERSION 3.15)
project(YDB-CPP-SDK LANGUAGES C CXX ASM)

set(BUILD_SHARED_LIBS Off)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS On)
set(YDB_MAX_LINK_JOBS 5 CACHE STRING "Maximum parallel link jobs for Ninja generator")
set_property(GLOBAL PROPERTY JOB_POOLS link_jobs=${YDB_MAX_LINK_JOBS})
set(CMAKE_JOB_POOL_LINK link_jobs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/cmake)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
enable_testing()

# Disable 'empty CUDA_ARCHITECTURES not allowed' warning
# Can't set it in cuda.cmake because of CMake policy subdirectory stack rules
cmake_policy(SET CMP0104 OLD)

include(cmake/common.cmake)
include(cmake/global_flags.cmake)
include(cmake/masm.cmake)
include(cmake/protobuf.cmake)
include(cmake/global_vars.cmake)

find_package(IDN REQUIRED)
find_package(Iconv REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

add_subdirectory(tools)
add_subdirectory(contrib)
add_subdirectory(library)
add_subdirectory(util)
add_subdirectory(certs)
add_subdirectory(ydb)
add_subdirectory(client)
add_subdirectory(examples)
