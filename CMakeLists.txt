cmake_minimum_required(VERSION 3.5)

file(READ "src/version.h" YDB_SDK_VERSION_FILE_RAW)
string(REGEX MATCH "YDB_SDK_VERSION = \"([0-9]+\\.[0-9]+\\.[0-9]+)\"" _ ${YDB_SDK_VERSION_FILE_RAW})
set(YDB_SDK_VERSION ${CMAKE_MATCH_1})
message(STATUS "YDB C++ SDK version: ${YDB_SDK_VERSION}")

project(YDB-CPP-SDK VERSION ${YDB_SDK_VERSION} LANGUAGES C CXX ASM)

option(YDB_SDK_INSTALL "Install YDB C++ SDK" Off)
option(YDB_SDK_TESTS "Build YDB C++ SDK tests" Off)
option(YDB_SDK_EXAMPLES "Build YDB C++ SDK examples" On)
option(YDB_SDK_ENABLE_OTEL_METRICS "Build OpenTelemetry metrics plugin" Off)
option(YDB_SDK_ENABLE_OTEL_TRACE "Build OpenTelemetry trace plugin" Off)
option(YDB_SDK_ODBC "Build YDB ODBC driver" Off)
if (YDB_SDK_ODBC)
  # ODBC driver is a shared library; static archives linked into it must be PIC.
  set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)
endif()
set(YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET "" CACHE STRING "Name of cmake target preparing google common proto library")
option(YDB_SDK_USE_RAPID_JSON "Search for rapid json library in system" ON)

set(BUILD_SHARED_LIBS Off)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS On)

set(YDB_SDK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(YDB_SDK_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(YDB-CPP-SDK_AVAILABLE_COMPONENTS "" CACHE INTERNAL "")
set(YDB-CPP-SDK_COMPONENT_TARGETS "" CACHE INTERNAL "")

#[=============================================================================[
  NOTE: if `ccache` is used with the environment variable `CCACHE_BASEDIR`,
  these cached variable should be set manually by passing them to `cmake` as
  `-DARCADIA_ROOT=source/path/relative/to/build/dir` and
  `-DARCADIA_BUILD_ROOT=.`, because in that case the macro `__FILE__` will be
  expanded to a relative path, even if the source code file was specified as
  an absolute path, and we have to know the proper prefix of that path.
  See details: https://ccache.dev/manual/3.1.html#_compiling_in_different_directories
#]=============================================================================]
set(ARCADIA_ROOT ${YDB_SDK_SOURCE_DIR} CACHE INTERNAL "The source root directory")
set(ARCADIA_BUILD_ROOT ${YDB_SDK_BINARY_DIR} CACHE INTERNAL "The build root directory")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

list(APPEND CMAKE_MODULE_PATH ${YDB_SDK_SOURCE_DIR}/cmake)

include(cmake/global_flags.cmake)
include(cmake/global_vars.cmake)
include(cmake/install.cmake)
include(cmake/common.cmake)
include(cmake/ccache.cmake)
include(cmake/protobuf.cmake)
include(cmake/external_libs.cmake)

if (YDB_SDK_TESTS)
  include(cmake/testing.cmake)
endif()

add_subdirectory(tools)
add_subdirectory(contrib/libs)
add_subdirectory(library/cpp)
add_subdirectory(include/ydb-cpp-sdk/client)
add_subdirectory(src)
add_subdirectory(util)
add_subdirectory(plugins)

#_ydb_sdk_validate_public_headers()

if (YDB_SDK_ODBC)
  add_subdirectory(odbc)
endif()

if (YDB_SDK_EXAMPLES)
  add_subdirectory(examples)
endif()

if (YDB_SDK_TESTS)
  add_subdirectory(tests)
endif()

if (YDB_SDK_INSTALL)
  function(_ydb_sdk_create_package_library PackageProp TargetName ComponentName)
    foreach(Tgt IN LISTS YDB_CPP_${PackageProp}_COMPONENT_TARGETS)
      _ydb_sdk_collect_package_target(${PackageProp} ${Tgt})
    endforeach()

    get_property(PackageSources GLOBAL PROPERTY YDB_CPP_${PackageProp}_SOURCES)
    if (NOT PackageSources)
      return()
    endif()

    get_property(PackageInternalDeps GLOBAL PROPERTY YDB_CPP_${PackageProp}_INTERNAL_DEPS)
    get_property(PackagePublicDeps GLOBAL PROPERTY YDB_CPP_${PackageProp}_PUBLIC_DEPS)
    get_property(PackagePublicOpts GLOBAL PROPERTY YDB_CPP_${PackageProp}_PUBLIC_OPTS)
    get_property(PackagePublicDefs GLOBAL PROPERTY YDB_CPP_${PackageProp}_PUBLIC_DEFS)
    get_property(PackageIncludeDirs GLOBAL PROPERTY YDB_CPP_${PackageProp}_INCLUDE_DIRS)

    list(REMOVE_DUPLICATES PackageSources)
    if (PackageInternalDeps)
      list(REMOVE_DUPLICATES PackageInternalDeps)
    endif()
    if (PackagePublicDeps)
      list(REMOVE_DUPLICATES PackagePublicDeps)
      list(REMOVE_ITEM PackagePublicDeps ${PackageInternalDeps})
    endif()
    if (PackagePublicOpts)
      list(REMOVE_DUPLICATES PackagePublicOpts)
    endif()
    if (PackagePublicDefs)
      list(REMOVE_DUPLICATES PackagePublicDefs)
    endif()
    if (PackageIncludeDirs)
      list(REMOVE_DUPLICATES PackageIncludeDirs)
    endif()

    foreach(src IN LISTS PackageSources)
      if (NOT EXISTS "${src}")
        set_source_files_properties("${src}" PROPERTIES GENERATED TRUE)
      endif()
    endforeach()

    add_library(${TargetName} STATIC ${PackageSources})
    target_include_directories(${TargetName} PUBLIC
      $<BUILD_INTERFACE:${YDB_SDK_SOURCE_DIR}>
      $<BUILD_INTERFACE:${YDB_SDK_BINARY_DIR}>
      $<BUILD_INTERFACE:${YDB_SDK_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/__ydb_sdk_special_headers>
    )
    target_include_directories(${TargetName} PRIVATE
      ${PackageIncludeDirs}
    )
    if (TargetName STREQUAL "libydb-cpp")
      set_target_properties(${TargetName} PROPERTIES OUTPUT_NAME ydb-cpp)
    endif()
    target_compile_definitions(${TargetName} PUBLIC YDB_SDK_OSS ${PackagePublicDefs})
    target_link_libraries(${TargetName} PUBLIC ${PackagePublicDeps})
    if (PackageInternalDeps)
      add_dependencies(${TargetName} ${PackageInternalDeps})
    endif()
    if (PackagePublicOpts)
      target_compile_options(${TargetName} PUBLIC ${PackagePublicOpts})
    endif()
    _ydb_sdk_install_targets(TARGETS ${TargetName} COMPONENT ${ComponentName})
  endfunction()

  _ydb_sdk_create_package_library(CORE libydb-cpp libydb-cpp)
  _ydb_sdk_create_package_library(LIBYDB_CPP_IAM ydb-cpp-iam libydb-cpp-iam)
  _ydb_sdk_create_package_library(LIBYDB_CPP_OTEL_METRICS ydb-cpp-otel-metrics libydb-cpp-otel-metrics)
  _ydb_sdk_create_package_library(LIBYDB_CPP_OTEL_TRACING ydb-cpp-otel-tracing libydb-cpp-otel-tracing)

  _ydb_sdk_install_headers(${CMAKE_INSTALL_INCLUDEDIR})
  install(EXPORT ydb-cpp-sdk-targets
    FILE ydb-cpp-sdk-targets.cmake
    CONFIGURATIONS RELEASE
    NAMESPACE YDB-CPP-SDK::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ydb-cpp-sdk/release
    COMPONENT libydb-cpp
  )
  configure_package_config_file(
    ${YDB_SDK_SOURCE_DIR}/cmake/ydb-cpp-sdk-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ydb-cpp-sdk-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ydb-cpp-sdk
  )
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ydb-cpp-sdk-config-version.cmake
    VERSION ${YDB_SDK_VERSION} COMPATIBILITY SameMajorVersion
  )
  _ydb_sdk_directory_install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ydb-cpp-sdk-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ydb-cpp-sdk-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ydb-cpp-sdk
  )
  _ydb_sdk_directory_install(FILES
    ${YDB_SDK_SOURCE_DIR}/cmake/FindBrotli.cmake
    ${YDB_SDK_SOURCE_DIR}/cmake/FindIDN.cmake
    ${YDB_SDK_SOURCE_DIR}/cmake/FindLZ4.cmake
    ${YDB_SDK_SOURCE_DIR}/cmake/FindxxHash.cmake
    ${YDB_SDK_SOURCE_DIR}/cmake/FindZSTD.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ydb-cpp-sdk/Modules
  )

  include(cmake/PackSDK.cmake)
endif()
