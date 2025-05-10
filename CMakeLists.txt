cmake_minimum_required(VERSION 3.22)

file(READ "src/version.h" YDB_SDK_VERSION_FILE_RAW)
string(REGEX MATCH "YDB_SDK_VERSION = \"([0-9]+\\.[0-9]+\\.[0-9]+)\"" _ ${YDB_SDK_VERSION_FILE_RAW})
set(YDB_SDK_VERSION ${CMAKE_MATCH_1})
message(STATUS "YDB C++ SDK version: ${YDB_SDK_VERSION}")

project(ydb-cpp-sdk VERSION ${YDB_SDK_VERSION} LANGUAGES C CXX ASM)

option(YDB_SDK_INSTALL "Install the YDB C++ SDK and its CPM-managed dependencies" Off)
option(YDB_SDK_TESTS "Build all YDB C++ SDK test binaries" Off)
option(YDB_SDK_EXAMPLES "Build YDB C++ SDK examples" On)
option(YDB_SDK_ENABLE_OTEL_METRICS "Build the OpenTelemetry metrics plugin" Off)
option(YDB_SDK_ENABLE_OTEL_TRACE "Build the OpenTelemetry tracing plugin" Off)
option(YDB_CPP_SDK_SLO_USE_INSTALLED_SDK "Build only SLO workloads against one installed SDK prefix" Off)
option(YDB_SDK_ODBC "Build YDB ODBC driver" On)
set(YDB_SDK_DEPENDENCY_MODE "CPM" CACHE STRING
  "Dependency provider: CPM (default, pinned sources) or SYSTEM (Ubuntu DEB builds)")
set_property(CACHE YDB_SDK_DEPENDENCY_MODE PROPERTY STRINGS CPM SYSTEM)
string(TOUPPER "${YDB_SDK_DEPENDENCY_MODE}" YDB_SDK_DEPENDENCY_MODE)
if(NOT YDB_SDK_DEPENDENCY_MODE MATCHES "^(CPM|SYSTEM)$")
  message(FATAL_ERROR
    "YDB_SDK_DEPENDENCY_MODE must be CPM or SYSTEM, got '${YDB_SDK_DEPENDENCY_MODE}'")
endif()
set(YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET "" CACHE STRING
  "Advanced override: existing target that provides Google API common protos")
mark_as_advanced(YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET)
option(YDB_SDK_USE_RAPID_JSON "Enable JSON support through RapidJSON" ON)

set(BUILD_SHARED_LIBS Off)
if(YDB_SDK_INSTALL)
  set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME libydb-cpp)
endif()
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
include(cmake/coverage.cmake)
include(cmake/global_vars.cmake)
include(cmake/install.cmake)
include(cmake/common.cmake)
include(cmake/ccache.cmake)
include(cmake/protobuf.cmake)

if (YDB_CPP_SDK_SLO_USE_INSTALLED_SDK)
  set(YDB_SDK_DEPENDENCIES_HDR_ONLY ON)
  include(cmake/dependencies.cmake)
  include(cmake/testing.cmake)

  find_package(ydb-cpp-sdk REQUIRED COMPONENTS
    Coordination
    Driver
    FederatedTopic
    Iam
    OpenTelemetryMetrics
    Operation
    Result
    Scheme
    SolomonStats
    Table
    Topic
    Types
  )

  add_subdirectory(tools)
  add_subdirectory(contrib/libs)
  _ydb_sdk_prepare_imported_libc_compat(contrib-libs-libc_compat)
  add_subdirectory(src/library/string_utils/base64)
  add_subdirectory(library/cpp)
  add_subdirectory(util)
  target_sources(yutil PRIVATE "${YDB_SDK_SOURCE_DIR}/cmake/tstring_abi.cpp")
  add_subdirectory(tests/slo_workloads)

  return()
endif()

include(cmake/external_libs.cmake)

if (YDB_SDK_TESTS)
  include(cmake/testing.cmake)
endif()

set(_ydb_sdk_tests "${YDB_SDK_TESTS}")
set(YDB_SDK_TESTS OFF)
add_subdirectory(tools)
add_subdirectory(contrib/libs)
_ydb_sdk_prepare_imported_libc_compat(contrib-libs-libc_compat)
add_subdirectory(library/cpp)
add_subdirectory(include/ydb-cpp-sdk/client)
add_subdirectory(src)
add_subdirectory(util)
target_sources(yutil PRIVATE "${YDB_SDK_SOURCE_DIR}/cmake/tstring_abi.cpp")
add_subdirectory(plugins)
add_subdirectory(c_api)
set(YDB_SDK_TESTS "${_ydb_sdk_tests}")
unset(_ydb_sdk_tests)

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
    if (NOT TargetName STREQUAL "libydb-cpp")
      get_property(CoreSources GLOBAL PROPERTY YDB_CPP_CORE_SOURCES)
      if (CoreSources)
        list(REMOVE_DUPLICATES CoreSources)
        list(REMOVE_ITEM PackageSources ${CoreSources})
      endif()
    endif()
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
      _ydb_sdk_prepare_imported_libc_compat(${TargetName})
    endif()
    target_compile_definitions(${TargetName} PUBLIC YDB_SDK_OSS ${PackagePublicDefs})
    if (NOT TargetName STREQUAL "libydb-cpp")
      target_link_libraries(${TargetName} PUBLIC libydb-cpp)
    endif()
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

  install(EXPORT ydb-cpp-sdk-core-targets
    FILE ydb-cpp-sdk-core-targets.cmake
    CONFIGURATIONS RELEASE
    NAMESPACE ydb-cpp-sdk::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ydb-cpp-sdk/release
    COMPONENT libydb-cpp
  )
  if (TARGET ydb-cpp-iam)
    install(EXPORT ydb-cpp-sdk-iam-targets
      FILE ydb-cpp-sdk-iam-targets.cmake
      CONFIGURATIONS RELEASE
      NAMESPACE YDB-CPP-SDK::
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ydb-cpp-sdk/release
      COMPONENT libydb-cpp-iam
    )
  endif()
  if (TARGET ydb-cpp-otel-metrics)
    install(EXPORT ydb-cpp-sdk-otel-metrics-targets
      FILE ydb-cpp-sdk-otel-metrics-targets.cmake
      CONFIGURATIONS RELEASE
      NAMESPACE YDB-CPP-SDK::
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ydb-cpp-sdk/release
      COMPONENT libydb-cpp-otel-metrics
    )
  endif()
  if (TARGET ydb-cpp-otel-tracing)
    install(EXPORT ydb-cpp-sdk-otel-tracing-targets
      FILE ydb-cpp-sdk-otel-tracing-targets.cmake
      CONFIGURATIONS RELEASE
      NAMESPACE YDB-CPP-SDK::
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ydb-cpp-sdk/release
      COMPONENT libydb-cpp-otel-tracing
    )
  endif()

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
