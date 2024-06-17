cmake_minimum_required(VERSION 3.15)
project(YDB-CPP-SDK LANGUAGES C CXX ASM)

option(YDB_SDK_INSTALL Off)
option(YDB_SDK_TESTS Off)
option(YDB_SDK_EXAMPLES On)

set(BUILD_SHARED_LIBS Off)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS On)

set(YDB_SDK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(YDB_SDK_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH ${YDB_SDK_SOURCE_DIR}/cmake)
include_directories(${YDB_SDK_SOURCE_DIR} ${YDB_SDK_SOURCE_DIR}/include ${YDB_SDK_BINARY_DIR})


include(GNUInstallDirs)
include(cmake/global_flags.cmake)
include(cmake/global_vars.cmake)
include(cmake/common.cmake)
include(cmake/ccache.cmake)
include(cmake/protobuf.cmake)
include(cmake/testing.cmake)
include(cmake/install.cmake)
include(cmake/external_libs.cmake)

add_subdirectory(tools)
add_subdirectory(contrib)
add_subdirectory(src)

if (YDB_SDK_EXAMPLES)
  add_subdirectory(examples)
endif()

if (YDB_SDK_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if (YDB_SDK_INSTALL)
  _ydb_sdk_directory_install(DIRECTORY ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  install(EXPORT ydb-cpp-sdk-targets
    CONFIGURATIONS RELEASE
    NAMESPACE YDB-CPP-SDK::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ydb-cpp-sdk/release
  )
endif()
