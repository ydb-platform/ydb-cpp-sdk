include_guard(GLOBAL)

# This file is the authoritative dependency manifest. Keep version changes here
# so presets, containers, workflows, and documentation do not drift apart.
set(YDB_SDK_ABSEIL_VERSION 20240116.0)
set(YDB_SDK_PROTOBUF_VERSION 26.1)
set(YDB_SDK_GRPC_REVISION e55f69cedd0ef7344e0bcb64b5ec9205e6aa4f04)
set(YDB_SDK_GRPC_SHA256 acdf93cc2d116769175712de3606f0ec6a4989f49276ca894b3ee398b1954015)
set(YDB_SDK_BASE64_VERSION 0.5.2)
set(YDB_SDK_BROTLI_VERSION 1.1.0)
set(YDB_SDK_JWT_CPP_VERSION 0.6.0)
set(YDB_SDK_OPENTELEMETRY_VERSION 1.26.0)
set(YDB_SDK_GOOGLETEST_VERSION 1.15.2)
set(YDB_SDK_HDR_HISTOGRAM_VERSION 0.11.8)
set(YDB_SDK_TBB_VERSION 2023.1.0)

set(YDB_SDK_ZLIB_VERSION 1.3.1)
set(YDB_SDK_XXHASH_VERSION 0.8.3)
set(YDB_SDK_ZSTD_VERSION 1.5.7)
set(YDB_SDK_BZIP2_VERSION 1.0.8)
set(YDB_SDK_LZ4_VERSION 1.10.0)
set(YDB_SDK_SNAPPY_VERSION 1.2.1)
set(YDB_SDK_DOUBLE_CONVERSION_VERSION 3.3.0)
set(YDB_SDK_RAPIDJSON_VERSION 1.1.0)
set(YDB_SDK_CARES_VERSION 1.27.0)
set(YDB_SDK_RE2_VERSION 2023-11-01)
set(YDB_SDK_CURL_VERSION 8.19.0)
set(YDB_SDK_OTEL_PROTO_VERSION 1.8.0)
string(REPLACE "." "_" YDB_SDK_CARES_GIT_VERSION "${YDB_SDK_CARES_VERSION}")

set(YDB_SDK_GOOGLEAPIS_GIT_TAG 3332dec527759859840a3a2ff108c67a54708130)
set(YDB_SDK_FASTLZ_GIT_TAG 344eb4025f9ae866ebf7a2ec48850f7113a97a42)

set(CPM_USE_NAMED_CACHE_DIRECTORIES ON CACHE BOOL
    "Use stable per-package directories in CPM_SOURCE_CACHE")
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING
    "Minimum CMake policy version accepted by fetched dependencies")
include("${CMAKE_CURRENT_LIST_DIR}/CPM.cmake")

set(_YDB_SDK_CPM_EXCLUDE_FROM_ALL YES)
set(_YDB_SDK_CPM_DISABLE_INSTALL YES)
if(YDB_SDK_INSTALL)
  set(_YDB_SDK_CPM_EXCLUDE_FROM_ALL NO)
  set(_YDB_SDK_CPM_DISABLE_INSTALL NO)
endif()

function(_ydb_sdk_cpm_package_stub package_name contents)
  set(_stub_dir "${CMAKE_BINARY_DIR}/cpm-package-stubs/${package_name}")
  file(MAKE_DIRECTORY "${_stub_dir}")
  file(WRITE "${_stub_dir}/${package_name}Config.cmake" "${contents}\n")
  set(${package_name}_DIR "${_stub_dir}" CACHE PATH "CPM package adapter" FORCE)
endfunction()

function(_ydb_sdk_alias_library alias_name)
  if(TARGET "${alias_name}")
    return()
  endif()
  foreach(_candidate IN LISTS ARGN)
    if(TARGET "${_candidate}")
      add_library("${alias_name}" INTERFACE IMPORTED GLOBAL)
      set_property(TARGET "${alias_name}" PROPERTY
        INTERFACE_LINK_LIBRARIES "${_candidate}")
      return()
    endif()
  endforeach()
  message(FATAL_ERROR "CPM dependency did not provide a target for ${alias_name}")
endfunction()

function(ydb_sdk_fetch_googleapis)
  if(DEFINED YDB_SDK_GOOGLEAPIS_SOURCE_DIR AND
     IS_DIRECTORY "${YDB_SDK_GOOGLEAPIS_SOURCE_DIR}/google")
    set(YDB_SDK_GOOGLEAPIS_SOURCE_DIR "${YDB_SDK_GOOGLEAPIS_SOURCE_DIR}" PARENT_SCOPE)
    return()
  endif()
  CPMAddPackage(
    NAME google_api_common_protos
    GITHUB_REPOSITORY googleapis/api-common-protos
    GIT_TAG ${YDB_SDK_GOOGLEAPIS_GIT_TAG}
    DOWNLOAD_ONLY YES
  )
  set(YDB_SDK_GOOGLEAPIS_SOURCE_DIR "${google_api_common_protos_SOURCE_DIR}" PARENT_SCOPE)
endfunction()

function(ydb_sdk_fetch_hdr_histogram)
  set(_saved_skip_install_rules "${CMAKE_SKIP_INSTALL_RULES}")
  set(CMAKE_SKIP_INSTALL_RULES ON)
  CPMAddPackage(
    NAME hdr_histogram
    GITHUB_REPOSITORY HdrHistogram/HdrHistogram_c
    GIT_TAG ${YDB_SDK_HDR_HISTOGRAM_VERSION}
    EXCLUDE_FROM_ALL YES
    OPTIONS
      "HDR_HISTOGRAM_BUILD_PROGRAMS OFF"
      "HDR_HISTOGRAM_BUILD_SHARED OFF"
      "HDR_LOG_REQUIRED OFF"
  )
  set(CMAKE_SKIP_INSTALL_RULES "${_saved_skip_install_rules}")
endfunction()

if(YDB_SDK_DEPENDENCIES_GOOGLEAPIS_ONLY)
  ydb_sdk_fetch_googleapis()
  return()
endif()

if(YDB_SDK_DEPENDENCIES_HDR_ONLY)
  ydb_sdk_fetch_hdr_histogram()
  return()
endif()

if(YDB_SDK_DEPENDENCY_MODE STREQUAL "SYSTEM")
  find_package(Protobuf CONFIG QUIET)
  if(NOT Protobuf_FOUND)
    find_package(Protobuf MODULE REQUIRED)
  endif()
  find_package(gRPC 1.41.0 REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(xxHash REQUIRED)
  find_package(ZSTD REQUIRED)
  find_package(BZip2 REQUIRED)
  find_package(LZ4 REQUIRED)
  find_package(Snappy 1.1.8 REQUIRED)
  find_package(Brotli 1.1.0 REQUIRED)
  find_package(double-conversion REQUIRED)
  find_package(TBB REQUIRED COMPONENTS tbb)
  if(YDB_SDK_USE_RAPID_JSON)
    find_package(RapidJSON REQUIRED)
    if(NOT TARGET RapidJSON::RapidJSON)
      add_library(RapidJSON::RapidJSON INTERFACE IMPORTED)
      target_include_directories(RapidJSON::RapidJSON INTERFACE ${RAPIDJSON_INCLUDE_DIRS})
    endif()
  endif()
else()
  set(_ydb_sdk_saved_install_component "${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}")
  set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME libydb-cpp)
  set(BUILD_SHARED_LIBS OFF)

  CPMAddPackage(
    NAME ZLIB
    GITHUB_REPOSITORY madler/zlib
    GIT_TAG v${YDB_SDK_ZLIB_VERSION}
    EXCLUDE_FROM_ALL YES
    OPTIONS "ZLIB_BUILD_EXAMPLES OFF"
  )
  if(YDB_SDK_INSTALL)
    install(TARGETS zlibstatic ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT libydb-cpp)
    install(FILES "${ZLIB_SOURCE_DIR}/zlib.h" "${ZLIB_BINARY_DIR}/zconf.h"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT libydb-cpp)
  endif()
  # Keep build-tree implementation targets out of protobuf/gRPC install
  # exports. Consumers resolve this conventional target through FindZLIB.
  if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB INTERFACE IMPORTED GLOBAL)
    set_target_properties(ZLIB::ZLIB PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_SOURCE_DIR};${ZLIB_BINARY_DIR}"
      INTERFACE_LINK_LIBRARIES zlibstatic)
    add_dependencies(ZLIB::ZLIB zlibstatic)
  endif()
  _ydb_sdk_cpm_package_stub(ZLIB
    "set(ZLIB_FOUND TRUE)\nset(ZLIB_VERSION_STRING ${YDB_SDK_ZLIB_VERSION})\nset(ZLIB_LIBRARIES ZLIB::ZLIB)\nset(ZLIB_LIBRARY ZLIB::ZLIB)\nset(ZLIB_INCLUDE_DIRS \"${ZLIB_SOURCE_DIR};${ZLIB_BINARY_DIR}\")\nset(ZLIB_INCLUDE_DIR \"${ZLIB_SOURCE_DIR}\")")

  CPMAddPackage(
    NAME abseil-cpp
    GITHUB_REPOSITORY abseil/abseil-cpp
    GIT_TAG ${YDB_SDK_ABSEIL_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS
      "ABSL_PROPAGATE_CXX_STD ON"
      "ABSL_ENABLE_INSTALL ${YDB_SDK_INSTALL}"
      "ABSL_BUILD_TESTING OFF"
  )
  if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Keep Abseil's architecture flag pairs intact after CMake de-duplication.
    foreach(_target IN ITEMS absl_random_internal_randen_hwaes
        absl_random_internal_randen_hwaes_impl)
      get_target_property(_copts ${_target} COMPILE_OPTIONS)
      list(REMOVE_ITEM _copts
        -Xarch_x86_64 -maes -msse4.1 -Xarch_arm64 -march=armv8-a+crypto)
      list(APPEND _copts
        "SHELL:-Xarch_x86_64 -maes"
        "SHELL:-Xarch_x86_64 -msse4.1"
        "SHELL:-Xarch_arm64 -march=armv8-a+crypto")
      set_property(TARGET ${_target} PROPERTY COMPILE_OPTIONS "${_copts}")
    endforeach()
  endif()
  _ydb_sdk_cpm_package_stub(absl "set(absl_FOUND TRUE)")

  CPMAddPackage(
    NAME Protobuf
    GITHUB_REPOSITORY protocolbuffers/protobuf
    GIT_TAG v${YDB_SDK_PROTOBUF_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS
      "protobuf_BUILD_TESTS OFF"
      "protobuf_BUILD_EXAMPLES OFF"
      "protobuf_BUILD_SHARED_LIBS OFF"
      "protobuf_INSTALL ${YDB_SDK_INSTALL}"
      "protobuf_ABSL_PROVIDER package"
      "protobuf_WITH_ZLIB ON"
      "utf8_range_ENABLE_INSTALL ${YDB_SDK_INSTALL}"
  )
  set(YDB_SDK_PROTOBUF_SOURCE_DIR "${Protobuf_SOURCE_DIR}")
  _ydb_sdk_alias_library(protobuf::libprotobuf libprotobuf)
  if(NOT TARGET protobuf::libprotoc AND TARGET libprotoc)
    add_library(protobuf::libprotoc ALIAS libprotoc)
  endif()
  if(NOT TARGET protobuf::protoc AND TARGET protoc)
    add_executable(protobuf::protoc ALIAS protoc)
  endif()
  _ydb_sdk_cpm_package_stub(Protobuf
    "set(Protobuf_FOUND TRUE)\nset(Protobuf_VERSION ${YDB_SDK_PROTOBUF_VERSION})\nset(Protobuf_LIBRARIES protobuf::libprotobuf)\nset(Protobuf_PROTOC_EXECUTABLE \"$<TARGET_FILE:protobuf::protoc>\")")

  # c-ares 1.27 only initializes this suffix in its shared-library branch.
  set(STATIC_SUFFIX _static)
  set(_ydb_sdk_saved_runtime_output "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  CPMAddPackage(
    NAME c-ares
    GITHUB_REPOSITORY c-ares/c-ares
    GIT_TAG cares-${YDB_SDK_CARES_GIT_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS
      "CARES_STATIC ON"
      "CARES_SHARED OFF"
      "CARES_BUILD_TESTS OFF"
      "CARES_BUILD_TOOLS OFF"
      "CARES_INSTALL ${YDB_SDK_INSTALL}"
  )
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_ydb_sdk_saved_runtime_output}")
  unset(STATIC_SUFFIX)
  _ydb_sdk_alias_library(c-ares::cares cares_static cares)
  _ydb_sdk_cpm_package_stub(c-ares "set(c-ares_FOUND TRUE)")

  set(_ydb_sdk_saved_skip_install_rules "${CMAKE_SKIP_INSTALL_RULES}")
  if(NOT YDB_SDK_INSTALL)
    set(CMAKE_SKIP_INSTALL_RULES ON)
  endif()
  CPMAddPackage(
    NAME re2
    GITHUB_REPOSITORY google/re2
    GIT_TAG ${YDB_SDK_RE2_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS
      "RE2_BUILD_TESTING OFF"
      "RE2_USE_ICU OFF"
  )
  set(CMAKE_SKIP_INSTALL_RULES "${_ydb_sdk_saved_skip_install_rules}")
  _ydb_sdk_alias_library(re2::re2 re2)
  _ydb_sdk_cpm_package_stub(re2 "set(re2_FOUND TRUE)")

  set(gRPC_INSTALL ${YDB_SDK_INSTALL} CACHE BOOL "" FORCE)
  set(gRPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(gRPC_BUILD_CODEGEN ON CACHE BOOL "" FORCE)
  # gRPC copies this property into protoc command arguments without
  # filtering an empty INSTALL_INTERFACE entry.
  get_target_property(_ydb_sdk_libprotoc_includes libprotoc INTERFACE_INCLUDE_DIRECTORIES)
  set_target_properties(libprotoc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${Protobuf_SOURCE_DIR}/src")
  CPMAddPackage(
    NAME gRPC
    URL https://github.com/grpc/grpc/archive/${YDB_SDK_GRPC_REVISION}.tar.gz
    URL_HASH SHA256=${YDB_SDK_GRPC_SHA256}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS
      "gRPC_INSTALL ${YDB_SDK_INSTALL}"
      "gRPC_BUILD_TESTS OFF"
      "gRPC_BUILD_CODEGEN ON"
      "gRPC_BUILD_GRPC_CSHARP_PLUGIN OFF"
      "gRPC_BUILD_GRPC_NODE_PLUGIN OFF"
      "gRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN OFF"
      "gRPC_BUILD_GRPC_PHP_PLUGIN OFF"
      "gRPC_BUILD_GRPC_PYTHON_PLUGIN OFF"
      "gRPC_BUILD_GRPC_RUBY_PLUGIN OFF"
      "gRPC_ZLIB_PROVIDER package"
      "gRPC_CARES_PROVIDER package"
      "gRPC_RE2_PROVIDER package"
      "gRPC_SSL_PROVIDER package"
      "gRPC_PROTOBUF_PROVIDER package"
      "gRPC_ABSL_PROVIDER package"
  )
  if(_ydb_sdk_libprotoc_includes)
    set_target_properties(libprotoc PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${_ydb_sdk_libprotoc_includes}")
  else()
    set_target_properties(libprotoc PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "")
  endif()
  if(TARGET grpc++ AND NOT TARGET gRPC::grpc++)
    add_library(gRPC::grpc++ ALIAS grpc++)
  endif()
  if(TARGET grpc_cpp_plugin AND NOT TARGET gRPC::grpc_cpp_plugin)
    add_executable(gRPC::grpc_cpp_plugin ALIAS grpc_cpp_plugin)
  endif()

  CPMAddPackage(
    NAME Brotli
    GITHUB_REPOSITORY google/brotli
    GIT_TAG v${YDB_SDK_BROTLI_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS "BROTLI_DISABLE_TESTS ON" "BROTLI_BUNDLED_MODE OFF"
  )
  _ydb_sdk_alias_library(Brotli::common brotlicommon-static brotlicommon)
  _ydb_sdk_alias_library(Brotli::dec brotlidec-static brotlidec)
  _ydb_sdk_alias_library(Brotli::enc brotlienc-static brotlienc)

  CPMAddPackage(
    NAME xxHash
    GITHUB_REPOSITORY Cyan4973/xxHash
    GIT_TAG v${YDB_SDK_XXHASH_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    SOURCE_SUBDIR cmake_unofficial
    OPTIONS "XXHASH_BUILD_XXHSUM OFF" "XXHASH_BUILD_ENABLE_INLINE_API OFF"
  )
  _ydb_sdk_alias_library(xxHash::xxHash xxhash xxhash_static)
  if(YDB_SDK_INSTALL)
    install(TARGETS xxhash ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT libydb-cpp)
    install(FILES "${xxHash_SOURCE_DIR}/xxhash.h"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT libydb-cpp)
  endif()

  CPMAddPackage(
    NAME zstd
    GITHUB_REPOSITORY facebook/zstd
    GIT_TAG v${YDB_SDK_ZSTD_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    SOURCE_SUBDIR build/cmake
    OPTIONS
      "ZSTD_BUILD_PROGRAMS OFF"
      "ZSTD_BUILD_TESTS OFF"
      "ZSTD_BUILD_SHARED OFF"
      "ZSTD_BUILD_STATIC ON"
  )
  _ydb_sdk_alias_library(ZSTD::ZSTD libzstd_static)

  CPMAddPackage(
    NAME bzip2
    GITHUB_REPOSITORY libarchive/bzip2
    GIT_TAG bzip2-${YDB_SDK_BZIP2_VERSION}
    DOWNLOAD_ONLY YES
  )
  add_library(ydb_sdk_bzip2 STATIC
    "${bzip2_SOURCE_DIR}/blocksort.c"
    "${bzip2_SOURCE_DIR}/huffman.c"
    "${bzip2_SOURCE_DIR}/crctable.c"
    "${bzip2_SOURCE_DIR}/randtable.c"
    "${bzip2_SOURCE_DIR}/compress.c"
    "${bzip2_SOURCE_DIR}/decompress.c"
    "${bzip2_SOURCE_DIR}/bzlib.c")
  set_target_properties(ydb_sdk_bzip2 PROPERTIES OUTPUT_NAME bz2)
  target_include_directories(ydb_sdk_bzip2 PUBLIC
    $<BUILD_INTERFACE:${bzip2_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  _ydb_sdk_alias_library(BZip2::BZip2 ydb_sdk_bzip2)
  if(YDB_SDK_INSTALL)
    install(TARGETS ydb_sdk_bzip2 ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT libydb-cpp)
    install(FILES "${bzip2_SOURCE_DIR}/bzlib.h"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT libydb-cpp)
  endif()

  CPMAddPackage(
    NAME lz4
    GITHUB_REPOSITORY lz4/lz4
    GIT_TAG v${YDB_SDK_LZ4_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    SOURCE_SUBDIR build/cmake
    OPTIONS
      "LZ4_BUILD_CLI OFF"
      "LZ4_BUILD_LEGACY_LZ4C OFF"
      "BUILD_SHARED_LIBS OFF"
  )
  _ydb_sdk_alias_library(LZ4::LZ4 lz4_static lz4)
  if(YDB_SDK_INSTALL)
    install(TARGETS lz4_static ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT libydb-cpp)
    install(FILES
      "${lz4_SOURCE_DIR}/lib/lz4.h"
      "${lz4_SOURCE_DIR}/lib/lz4frame.h"
      "${lz4_SOURCE_DIR}/lib/lz4hc.h"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT libydb-cpp)
  endif()

  CPMAddPackage(
    NAME Snappy
    GITHUB_REPOSITORY google/snappy
    GIT_TAG ${YDB_SDK_SNAPPY_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS
      "SNAPPY_BUILD_TESTS OFF"
      "SNAPPY_BUILD_BENCHMARKS OFF"
      "SNAPPY_INSTALL ON"
  )
  _ydb_sdk_alias_library(Snappy::snappy snappy)

  CPMAddPackage(
    NAME double-conversion
    GITHUB_REPOSITORY google/double-conversion
    GIT_TAG v${YDB_SDK_DOUBLE_CONVERSION_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS "BUILD_TESTING OFF"
  )
  _ydb_sdk_alias_library(double-conversion::double-conversion double-conversion)

  if(YDB_SDK_USE_RAPID_JSON)
    CPMAddPackage(
      NAME RapidJSON
      GITHUB_REPOSITORY Tencent/rapidjson
      GIT_TAG v${YDB_SDK_RAPIDJSON_VERSION}
      EXCLUDE_FROM_ALL YES
      OPTIONS
        "RAPIDJSON_BUILD_DOC OFF"
        "RAPIDJSON_BUILD_EXAMPLES OFF"
        "RAPIDJSON_BUILD_TESTS OFF"
        "RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF"
    )
    if(NOT TARGET RapidJSON::RapidJSON)
      add_library(RapidJSON::RapidJSON INTERFACE IMPORTED GLOBAL)
      target_include_directories(RapidJSON::RapidJSON INTERFACE
        "$<BUILD_INTERFACE:${RapidJSON_SOURCE_DIR}/include>")
    endif()
    _ydb_sdk_cpm_package_stub(RapidJSON
      "get_filename_component(_rapidjson_prefix \"\${CMAKE_CURRENT_LIST_DIR}/../../..\" ABSOLUTE)\nset(RapidJSON_FOUND TRUE)\nset(RapidJSON_INCLUDE_DIRS \"\${_rapidjson_prefix}/include\")\nunset(_rapidjson_prefix)")
    if(YDB_SDK_INSTALL)
      install(DIRECTORY "${RapidJSON_SOURCE_DIR}/include/rapidjson"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT libydb-cpp)
      install(FILES "${RapidJSON_DIR}/RapidJSONConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RapidJSON COMPONENT libydb-cpp)
    endif()
  endif()

  CPMAddPackage(
    NAME oneTBB
    GITHUB_REPOSITORY uxlfoundation/oneTBB
    GIT_TAG v${YDB_SDK_TBB_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS
      "TBB_TEST OFF"
      "TBB_EXAMPLES OFF"
      "TBB_STRICT OFF"
      "TBBMALLOC_BUILD OFF"
      "TBBMALLOC_PROXY_BUILD OFF"
      "TBB_INSTALL ${YDB_SDK_INSTALL}"
  )
  # oneTBB does not use C++ modules. Avoid requiring clang-scan-deps merely
  # because the SDK is configured as C++20 with a recent CMake and Clang.
  set_property(TARGET tbb PROPERTY CXX_SCAN_FOR_MODULES OFF)

  set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "${_ydb_sdk_saved_install_component}")
endif()

# Ubuntu packages do not provide the pinned base64 and jwt-cpp interfaces.
set(BASE64_WERROR OFF CACHE BOOL "" FORCE)
set(BASE64_BUILD_CLI OFF CACHE BOOL "" FORCE)
CPMAddPackage(NAME base64 GITHUB_REPOSITORY aklomp/base64
  GIT_TAG v${YDB_SDK_BASE64_VERSION}
  EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
  OPTIONS "BASE64_WERROR OFF" "BASE64_BUILD_CLI OFF")
_ydb_sdk_alias_library(aklomp::base64 base64)
_ydb_sdk_apply_aklomp_base64_simd_file_flags("${base64_SOURCE_DIR}")
CPMAddPackage(NAME jwt-cpp GITHUB_REPOSITORY Thalhammer/jwt-cpp
  GIT_TAG v${YDB_SDK_JWT_CPP_VERSION}
  EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
  OPTIONS "JWT_BUILD_EXAMPLES OFF" "JWT_BUILD_TESTS OFF")

if(YDB_SDK_TESTS)
  CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v${YDB_SDK_GOOGLETEST_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS "INSTALL_GTEST ${YDB_SDK_INSTALL}" "gtest_force_shared_crt ON"
  )
endif()

if(YDB_SDK_TESTS OR YDB_CPP_SDK_SLO_USE_INSTALLED_SDK)
  ydb_sdk_fetch_hdr_histogram()
endif()

if(YDB_SDK_ENABLE_OTEL_METRICS OR YDB_SDK_ENABLE_OTEL_TRACE OR YDB_SDK_TESTS)
  set(_ydb_sdk_otel_saved_install_component "${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}")
  if(YDB_SDK_INSTALL)
    set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME libydb-cpp-otel-metrics)
  endif()
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(WITH_OTLP_HTTP ON CACHE BOOL "" FORCE)
  set(WITH_OTLP_GRPC OFF CACHE BOOL "" FORCE)
  set(WITH_PROMETHEUS OFF CACHE BOOL "" FORCE)
  set(WITH_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(WITH_FUNC_TESTS OFF CACHE BOOL "" FORCE)
  set(WITH_BENCHMARK OFF CACHE BOOL "" FORCE)
  set(WITH_STL CXX20 CACHE STRING "" FORCE)
  set(WITH_ABI_VERSION_1 OFF CACHE BOOL "" FORCE)
  set(WITH_ABI_VERSION_2 ON CACHE BOOL "" FORCE)
  set(OPENTELEMETRY_INSTALL ${YDB_SDK_INSTALL} CACHE BOOL "" FORCE)

  string(REPLACE "." "_" _ydb_sdk_curl_tag "${YDB_SDK_CURL_VERSION}")
  CPMAddPackage(
    NAME curl
    GITHUB_REPOSITORY curl/curl
    GIT_TAG curl-${_ydb_sdk_curl_tag}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
    OPTIONS
      "BUILD_CURL_EXE OFF"
      "BUILD_LIBCURL_DOCS OFF"
      "BUILD_MISC_DOCS OFF"
      "CURL_DISABLE_INSTALL ${_YDB_SDK_CPM_DISABLE_INSTALL}"
      "CURL_USE_LIBPSL OFF"
      "CURL_USE_LIBSSH2 OFF"
      "CURL_ZLIB OFF"
      "CURL_BROTLI OFF"
      "CURL_ZSTD OFF"
      "HTTP_ONLY ON"
  )
  _ydb_sdk_cpm_package_stub(CURL
    "set(CURL_FOUND TRUE)\nset(CURL_VERSION ${YDB_SDK_CURL_VERSION})\nset(CURL_VERSION_STRING ${YDB_SDK_CURL_VERSION})")

  CPMAddPackage(
    NAME opentelemetry-proto
    GITHUB_REPOSITORY open-telemetry/opentelemetry-proto
    GIT_TAG v${YDB_SDK_OTEL_PROTO_VERSION}
    DOWNLOAD_ONLY YES
  )
  set(OTELCPP_PROTO_PATH "${opentelemetry-proto_SOURCE_DIR}" CACHE PATH "" FORCE)

  CPMAddPackage(
    NAME opentelemetry-cpp
    GITHUB_REPOSITORY open-telemetry/opentelemetry-cpp
    GIT_TAG v${YDB_SDK_OPENTELEMETRY_VERSION}
    EXCLUDE_FROM_ALL ${_YDB_SDK_CPM_EXCLUDE_FROM_ALL}
  )
  set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "${_ydb_sdk_otel_saved_install_component}")
endif()

if(NOT YDB_SDK_DEPENDENCY_MODE STREQUAL "SYSTEM")
  ydb_sdk_fetch_googleapis()
endif()

CPMAddPackage(
  NAME FastLZSource
  GITHUB_REPOSITORY ariya/FastLZ
  GIT_TAG ${YDB_SDK_FASTLZ_GIT_TAG}
  DOWNLOAD_ONLY YES
)
set(YDB_SDK_FASTLZ_SOURCE_DIR "${FastLZSource_SOURCE_DIR}")

# The SDK install archives link these separately installed packages; package
# collection must not compile their sources into the SDK a second time.
foreach(_ydb_sdk_cpm_package IN ITEMS
    ZLIB abseil-cpp Protobuf c-ares re2 gRPC Brotli xxHash zstd lz4 Snappy
    double-conversion RapidJSON oneTBB base64 jwt-cpp curl opentelemetry-cpp)
  if(DEFINED ${_ydb_sdk_cpm_package}_SOURCE_DIR)
    set_property(GLOBAL APPEND PROPERTY YDB_SDK_CPM_SOURCE_DIRS
      "${${_ydb_sdk_cpm_package}_SOURCE_DIR}")
  endif()
endforeach()
