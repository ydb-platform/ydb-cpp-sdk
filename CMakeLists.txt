cmake_minimum_required(VERSION 3.5)

file(READ "src/version.h" YDB_SDK_VERSION_FILE_RAW)
string(REGEX MATCH "YDB_SDK_VERSION = \"([0-9]+\\.[0-9]+\\.[0-9]+)\"" _ ${YDB_SDK_VERSION_FILE_RAW})
set(YDB_SDK_VERSION ${CMAKE_MATCH_1})
message(STATUS "YDB C++ SDK version: ${YDB_SDK_VERSION}")

project(YDB-CPP-SDK VERSION ${YDB_SDK_VERSION} LANGUAGES C CXX ASM)

option(YDB_SDK_INSTALL "Install YDB C++ SDK" Off)
option(YDB_SDK_TESTS "Build YDB C++ SDK tests" Off)
option(YDB_SDK_EXAMPLES "Build YDB C++ SDK examples" On)
set(YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET "" CACHE STRING "Name of cmake target preparing google common proto library")
option(YDB_SDK_USE_RAPID_JSON "Search for rapid json library in system" ON)

# Опции для систем мониторинга
option(YDB_SDK_MONITORING_PROMETHEUS "Enable Prometheus monitoring" OFF)
option(YDB_SDK_MONITORING_OPENTELEMETRY "Enable OpenTelemetry monitoring" OFF)
option(YDB_SDK_MONITORING_DATADOG "Enable Datadog monitoring" OFF)
option(YDB_SDK_MONITORING_NEWRELIC "Enable New Relic monitoring" OFF)
option(YDB_SDK_MONITORING_APPDYNAMICS "Enable AppDynamics monitoring" OFF)
option(YDB_SDK_MONITORING_VICTORIAMETRICS "Enable Victoria Metrics monitoring" OFF)
option(YDB_SDK_MONITORING_SOLOMON "Enable Yandex Solomon monitoring" ON)

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

#_ydb_sdk_validate_public_headers()

if (YDB_SDK_EXAMPLES)
  add_subdirectory(examples)
endif()

if (YDB_SDK_TESTS)
  add_subdirectory(tests)
endif()

if (YDB_SDK_INSTALL)
  _ydb_sdk_install_headers(${CMAKE_INSTALL_INCLUDEDIR})
  install(EXPORT ydb-cpp-sdk-targets
    FILE ydb-cpp-sdk-targets.cmake
    CONFIGURATIONS RELEASE
    NAMESPACE YDB-CPP-SDK::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ydb-cpp-sdk/release
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

# Поиск зависимостей для систем мониторинга
if(YDB_SDK_MONITORING_PROMETHEUS)
    find_package(Prometheus REQUIRED)
endif()

if(YDB_SDK_MONITORING_OPENTELEMETRY)
    find_package(OpenTelemetry REQUIRED)
endif()

if(YDB_SDK_MONITORING_DATADOG)
    find_package(Datadog REQUIRED)
endif()

if(YDB_SDK_MONITORING_NEWRELIC)
    find_package(NewRelic REQUIRED)
endif()

if(YDB_SDK_MONITORING_APPDYNAMICS)
    find_package(AppDynamics REQUIRED)
endif()

if(YDB_SDK_MONITORING_VICTORIAMETRICS)
    find_package(VictoriaMetrics REQUIRED)
endif()

# Поиск дополнительных зависимостей
find_package(Brotli REQUIRED)
find_package(LZ4 REQUIRED)
find_package(xxHash REQUIRED)
find_package(ZSTD REQUIRED)

# Определяем основную библиотеку
add_library(ydb-cpp-sdk STATIC
    src/client/common_client/settings.cpp
    src/client/driver/driver.cpp
)

# Добавляем зависимости в target_link_libraries
target_link_libraries(ydb-cpp-sdk
    PRIVATE
    client-ydb_common_client
    client-ydb_types-credentials
    impl-ydb_internal-common
    yutil
    protobuf::libprotobuf
    grpc++
    grpc++_reflection
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    brotli
    lz4
    xxhash
    zstd
    BZip2::BZip2
    IDN::IDN
    $<$<BOOL:${YDB_SDK_MONITORING_PROMETHEUS}>:prometheus-cpp::prometheus-cpp>
    $<$<BOOL:${YDB_SDK_MONITORING_OPENTELEMETRY}>:OpenTelemetry::OpenTelemetry>
    $<$<BOOL:${YDB_SDK_MONITORING_DATADOG}>:Datadog::Datadog>
    $<$<BOOL:${YDB_SDK_MONITORING_NEWRELIC}>:NewRelic::NewRelic>
    $<$<BOOL:${YDB_SDK_MONITORING_APPDYNAMICS}>:AppDynamics::AppDynamics>
    $<$<BOOL:${YDB_SDK_MONITORING_VICTORIAMETRICS}>:VictoriaMetrics::VictoriaMetrics>
)

target_include_directories(ydb-cpp-sdk
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
