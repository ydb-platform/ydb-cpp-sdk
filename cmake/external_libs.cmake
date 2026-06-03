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
    set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME libydb-cpp)
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
    set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME libydb-cpp)
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
    set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME libydb-cpp)
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
    set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME libydb-cpp)
    FetchContent_MakeAvailable(jwt-cpp)
  endif()
endif()
find_package(double-conversion REQUIRED)

# OpenTelemetry
set(_ydb_sdk_vendor_otel "${YDB_SDK_SOURCE_DIR}/third_party/opentelemetry-cpp")

if (YDB_SDK_ENABLE_OTEL_METRICS OR YDB_SDK_ENABLE_OTEL_TRACE)
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(WITH_OTLP_HTTP OFF CACHE BOOL "" FORCE)
  set(WITH_OTLP_GRPC OFF CACHE BOOL "" FORCE)
  set(WITH_PROMETHEUS OFF CACHE BOOL "" FORCE)
  set(WITH_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(OTELCPP_MAINTAINER_MODE OFF CACHE BOOL "" FORCE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)
  set(WITH_STL "CXX20" CACHE STRING "" FORCE)
  set(WITH_ABI_VERSION_1 OFF CACHE BOOL "" FORCE)
  set(WITH_ABI_VERSION_2 ON CACHE BOOL "" FORCE)
  set(OPENTELEMETRY_INSTALL ON CACHE BOOL "" FORCE)

  set(_ydb_sdk_saved_install_component "${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}")
  set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME libydb-cpp-otel-metrics)

  if (EXISTS "${_ydb_sdk_vendor_otel}/CMakeLists.txt")
    add_subdirectory("${_ydb_sdk_vendor_otel}" "${YDB_SDK_BINARY_DIR}/third_party/opentelemetry-cpp")
  else()
    find_package(opentelemetry-cpp QUIET)
    if (NOT opentelemetry-cpp_FOUND)
      if (FETCHCONTENT_FULLY_DISCONNECTED)
        message(FATAL_ERROR
          "opentelemetry-cpp is required but was not found (find_package), vendored sources are missing at "
          "'${_ydb_sdk_vendor_otel}', and FETCHCONTENT_FULLY_DISCONNECTED=ON prevents FetchContent.")
      else()
        include(FetchContent)
        FetchContent_Declare(
          opentelemetry-cpp
          GIT_REPOSITORY https://github.com/open-telemetry/opentelemetry-cpp.git
          GIT_TAG v1.26.0
        )
        FetchContent_MakeAvailable(opentelemetry-cpp)
      endif()
    endif()
  endif()

  if (NOT TARGET opentelemetry-cpp::api)
    if (TARGET opentelemetry_api)
      add_library(opentelemetry-cpp::api ALIAS opentelemetry_api)
    endif()
    if (TARGET opentelemetry_metrics)
      add_library(opentelemetry-cpp::metrics ALIAS opentelemetry_metrics)
    endif()
    if (TARGET opentelemetry_trace)
      add_library(opentelemetry-cpp::trace ALIAS opentelemetry_trace)
    endif()
    if (TARGET opentelemetry_common)
      add_library(opentelemetry-cpp::common ALIAS opentelemetry_common)
    endif()
    if (TARGET opentelemetry_resources)
      add_library(opentelemetry-cpp::resources ALIAS opentelemetry_resources)
    endif()
    if (TARGET opentelemetry_version)
      add_library(opentelemetry-cpp::version ALIAS opentelemetry_version)
    endif()
  endif()

  if (YDB_SDK_INSTALL)
    set(_ydb_sdk_otel_binary_dir "${YDB_SDK_BINARY_DIR}/third_party/opentelemetry-cpp")
    set(_ydb_sdk_otel_install_component libydb-cpp-otel-metrics)

    if (EXISTS "${_ydb_sdk_otel_binary_dir}/cmake/opentelemetry-cpp")
      install(DIRECTORY "${_ydb_sdk_otel_binary_dir}/cmake/opentelemetry-cpp/"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/opentelemetry-cpp"
        COMPONENT ${_ydb_sdk_otel_install_component}
        USE_SOURCE_PERMISSIONS)
    endif()

    if (EXISTS "${_ydb_sdk_vendor_otel}/cmake/find-package-support-functions.cmake")
      install(FILES "${_ydb_sdk_vendor_otel}/cmake/find-package-support-functions.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/opentelemetry-cpp"
        COMPONENT ${_ydb_sdk_otel_install_component})
    endif()

    foreach(_ydb_sdk_otel_cmake_file IN ITEMS
        component-definitions.cmake
        thirdparty-dependency-definitions.cmake)
      if (EXISTS "${_ydb_sdk_otel_binary_dir}/cmake/${_ydb_sdk_otel_cmake_file}")
        install(FILES "${_ydb_sdk_otel_binary_dir}/cmake/${_ydb_sdk_otel_cmake_file}"
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/opentelemetry-cpp"
          COMPONENT ${_ydb_sdk_otel_install_component})
      endif()
    endforeach()

    # Export *-target.cmake files are generated during the OTel build; glob at install time.
    install(CODE "
      file(GLOB _ydb_sdk_otel_export_dirs \"${_ydb_sdk_otel_binary_dir}/CMakeFiles/Export/*\")
      foreach(_ydb_sdk_otel_export_dir IN LISTS _ydb_sdk_otel_export_dirs)
        if (IS_DIRECTORY \"\${_ydb_sdk_otel_export_dir}\")
          file(GLOB _ydb_sdk_otel_export_cmake \"\${_ydb_sdk_otel_export_dir}/*.cmake\")
          if (_ydb_sdk_otel_export_cmake)
            file(INSTALL \${_ydb_sdk_otel_export_cmake}
              DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/opentelemetry-cpp\")
          endif()
        endif()
      endforeach()
    " COMPONENT ${_ydb_sdk_otel_install_component})

    foreach(_ydb_sdk_otel_include_dir IN ITEMS api sdk exporters)
      if (EXISTS "${_ydb_sdk_vendor_otel}/${_ydb_sdk_otel_include_dir}/include")
        install(DIRECTORY "${_ydb_sdk_vendor_otel}/${_ydb_sdk_otel_include_dir}/include/"
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
          COMPONENT ${_ydb_sdk_otel_install_component}
          USE_SOURCE_PERMISSIONS)
      endif()
    endforeach()

    install(CODE "
      file(GLOB_RECURSE _ydb_sdk_otel_static_libs
        \"${_ydb_sdk_otel_binary_dir}/libopentelemetry_*.a\")
      if (_ydb_sdk_otel_static_libs)
        file(INSTALL \${_ydb_sdk_otel_static_libs}
          DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}\")
      endif()
    " COMPONENT ${_ydb_sdk_otel_install_component})
  endif()

  set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "${_ydb_sdk_saved_install_component}")
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
