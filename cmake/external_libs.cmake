find_package(IDN REQUIRED)
find_package(Iconv REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC 1.41.0 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(xxHash REQUIRED)
find_package(ZSTD REQUIRED)
find_package(BZip2 REQUIRED)
find_package(LZ4 REQUIRED)
find_package(Snappy 1.1.8 REQUIRED)
set(_ydb_sdk_vendor_base64 "${YDB_SDK_SOURCE_DIR}/third_party/aklomp-base64")
set(_ydb_sdk_vendor_jwt_cpp "${YDB_SDK_SOURCE_DIR}/third_party/jwt-cpp")

find_package(base64 QUIET)
if (NOT base64_FOUND)
  if (EXISTS "${_ydb_sdk_vendor_base64}/CMakeLists.txt")
    set(BASE64_WERROR OFF CACHE BOOL "" FORCE)
    set(BASE64_BUILD_CLI OFF CACHE BOOL "" FORCE)
    add_subdirectory("${_ydb_sdk_vendor_base64}" "${YDB_SDK_BINARY_DIR}/third_party/aklomp-base64")
    _ydb_sdk_apply_aklomp_base64_simd_file_flags("${_ydb_sdk_vendor_base64}")
    if (TARGET base64 AND NOT TARGET aklomp::base64)
      add_library(aklomp::base64 ALIAS base64)
    endif()
  elseif (FETCHCONTENT_FULLY_DISCONNECTED)
    message(FATAL_ERROR
      "base64 is required but was not found (find_package), vendored sources are missing at "
      "'${_ydb_sdk_vendor_base64}', and FETCHCONTENT_FULLY_DISCONNECTED=ON prevents FetchContent.")
  else()
    include(FetchContent)
    FetchContent_Declare(
      base64
      GIT_REPOSITORY https://github.com/aklomp/base64.git
      GIT_TAG v0.5.2
    )
    FetchContent_MakeAvailable(base64)
    get_target_property(_ydb_sdk_b64_src_dir base64 SOURCE_DIR)
    _ydb_sdk_apply_aklomp_base64_simd_file_flags("${_ydb_sdk_b64_src_dir}")
    if (TARGET base64 AND NOT TARGET aklomp::base64)
      add_library(aklomp::base64 ALIAS base64)
    endif()
  endif()
endif()
find_package(Brotli 1.1.0 REQUIRED)
find_package(jwt-cpp QUIET)
if (NOT jwt-cpp_FOUND)
  if (EXISTS "${_ydb_sdk_vendor_jwt_cpp}/CMakeLists.txt")
    set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(JWT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory("${_ydb_sdk_vendor_jwt_cpp}" "${YDB_SDK_BINARY_DIR}/third_party/jwt-cpp")
  elseif (FETCHCONTENT_FULLY_DISCONNECTED)
    message(FATAL_ERROR
      "jwt-cpp is required but was not found (find_package), vendored sources are missing at "
      "'${_ydb_sdk_vendor_jwt_cpp}', and FETCHCONTENT_FULLY_DISCONNECTED=ON prevents FetchContent.")
  else()
    include(FetchContent)
    FetchContent_Declare(
      jwt-cpp
      GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
      GIT_TAG v0.6.0
    )
    set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(JWT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(jwt-cpp)
  endif()
endif()
find_package(double-conversion REQUIRED)

# OpenTelemetry
if (YDB_SDK_ENABLE_OTEL_METRICS OR YDB_SDK_ENABLE_OTEL_TRACE)
  find_package(opentelemetry-cpp REQUIRED)
endif()

# RapidJSON
if (YDB_SDK_USE_RAPID_JSON)
  find_package(RapidJSON REQUIRED)

  add_library(RapidJSON::RapidJSON INTERFACE IMPORTED)

  target_include_directories(RapidJSON::RapidJSON INTERFACE
    ${RAPIDJSON_INCLUDE_DIRS}
  )
endif()

# api-common-protos
option(YDB_SDK_USE_SYSTEM_GOOGLEAPIS "Use system-provided yandex-googleapis-api-common-protos" OFF)

if (YDB_SDK_USE_SYSTEM_GOOGLEAPIS)
  find_package(yandex-googleapis-api-common-protos REQUIRED)
  add_library(api-common-protos ALIAS yandex-googleapis-api-common-protos::api-common-protos)
elseif (YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET)
  add_library(api-common-protos ALIAS ${YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET})
else()
  file(MAKE_DIRECTORY ${YDB_SDK_BINARY_DIR}/third_party/api-common-protos)
  file(GLOB_RECURSE API_COMMON_PROTOS_SOURCES
    ${YDB_SDK_SOURCE_DIR}/third_party/api-common-protos/google/*.proto
  )

  _ydb_sdk_init_proto_library_impl(api-common-protos Off)

  set_property(TARGET api-common-protos PROPERTY
    PROTO_NAMESPACE third_party/api-common-protos
  )

  set_property(TARGET api-common-protos APPEND PROPERTY 
    PROTO_ADDINCL 
      ./third_party/api-common-protos
      ${YDB_SDK_SOURCE_DIR}
  )

  target_include_directories(api-common-protos PUBLIC
    $<BUILD_INTERFACE:${YDB_SDK_BINARY_DIR}/third_party/api-common-protos>
  )

  _ydb_sdk_gen_proto_messages(api-common-protos PRIVATE Off ${API_COMMON_PROTOS_SOURCES})

  _ydb_sdk_install_targets(TARGETS api-common-protos)
endif()

# FastLZ
add_library(FastLZ 
  ${YDB_SDK_SOURCE_DIR}/third_party/FastLZ/fastlz.c
)

target_include_directories(FastLZ PUBLIC
  $<BUILD_INTERFACE:${YDB_SDK_SOURCE_DIR}/third_party/FastLZ>
)

_ydb_sdk_install_targets(TARGETS FastLZ)

# nayuki_md5
add_library(nayuki_md5)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
target_sources(nayuki_md5 PRIVATE 
  ${YDB_SDK_SOURCE_DIR}/third_party/nayuki_md5/nayuki_md5-fast-x8664.S
)
else()
target_sources(nayuki_md5 PRIVATE 
  ${YDB_SDK_SOURCE_DIR}/third_party/nayuki_md5/nayuki_md5.c
)
endif()

target_include_directories(nayuki_md5 PUBLIC
  $<BUILD_INTERFACE:${YDB_SDK_SOURCE_DIR}/third_party/nayuki_md5>
  $<INSTALL_INTERFACE:third_party/nayuki_md5>
)

_ydb_sdk_install_targets(TARGETS nayuki_md5)
