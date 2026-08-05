# Platform prerequisites deliberately remain outside CPM.
find_package(IDN REQUIRED)
find_package(Iconv REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/dependencies.cmake")

# OpenTelemetry releases have used both plain and namespaced build-tree target
# names. The SDK consumes one stable spelling in either dependency mode.
if(YDB_SDK_ENABLE_OTEL_METRICS OR YDB_SDK_ENABLE_OTEL_TRACE OR YDB_SDK_TESTS)
  if(NOT TARGET opentelemetry-cpp::api AND TARGET opentelemetry_api)
    add_library(opentelemetry-cpp::api ALIAS opentelemetry_api)
  endif()
  if(NOT TARGET opentelemetry-cpp::metrics AND TARGET opentelemetry_metrics)
    add_library(opentelemetry-cpp::metrics ALIAS opentelemetry_metrics)
  endif()
  if(NOT TARGET opentelemetry-cpp::trace AND TARGET opentelemetry_trace)
    add_library(opentelemetry-cpp::trace ALIAS opentelemetry_trace)
  endif()
  if(NOT TARGET opentelemetry-cpp::common AND TARGET opentelemetry_common)
    add_library(opentelemetry-cpp::common ALIAS opentelemetry_common)
  endif()
  if(NOT TARGET opentelemetry-cpp::resources AND TARGET opentelemetry_resources)
    add_library(opentelemetry-cpp::resources ALIAS opentelemetry_resources)
  endif()
  if(NOT TARGET opentelemetry-cpp::version AND TARGET opentelemetry_version)
    add_library(opentelemetry-cpp::version ALIAS opentelemetry_version)
  endif()
endif()

# Google API common protos remain independently packageable for Ubuntu, while
# CPM builds generate the same library directly from the pinned source tree.
if(YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET)
  add_library(api-common-protos ALIAS ${YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET})
elseif(YDB_SDK_DEPENDENCY_MODE STREQUAL "SYSTEM")
  find_package(yandex-googleapis-api-common-protos REQUIRED)
  add_library(api-common-protos ALIAS
    yandex-googleapis-api-common-protos::api-common-protos)
else()
  file(GLOB_RECURSE API_COMMON_PROTOS_SOURCES CONFIGURE_DEPENDS
    "${YDB_SDK_GOOGLEAPIS_SOURCE_DIR}/google/*.proto")

  _ydb_sdk_add_library(api-common-protos)
  target_link_libraries(api-common-protos PUBLIC protobuf::libprotobuf)
  target_include_directories(api-common-protos PUBLIC
    $<BUILD_INTERFACE:${YDB_SDK_BINARY_DIR}/third_party/api-common-protos>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

  foreach(_proto IN LISTS API_COMMON_PROTOS_SOURCES)
    file(RELATIVE_PATH _proto_relative "${YDB_SDK_GOOGLEAPIS_SOURCE_DIR}" "${_proto}")
    string(REGEX REPLACE "\\.proto$" ".pb.cc" _proto_cc "${_proto_relative}")
    string(REGEX REPLACE "\\.proto$" ".pb.h" _proto_h "${_proto_relative}")
    set(_proto_cc "${YDB_SDK_BINARY_DIR}/third_party/api-common-protos/${_proto_cc}")
    set(_proto_h "${YDB_SDK_BINARY_DIR}/third_party/api-common-protos/${_proto_h}")
    get_filename_component(_proto_output_dir "${_proto_cc}" DIRECTORY)
    file(MAKE_DIRECTORY "${_proto_output_dir}")
    add_custom_command(
      OUTPUT "${_proto_cc}" "${_proto_h}"
      COMMAND protobuf::protoc
        "--cpp_out=${YDB_SDK_BINARY_DIR}/third_party/api-common-protos"
        "-I${YDB_SDK_GOOGLEAPIS_SOURCE_DIR}"
        "-I${YDB_SDK_PROTOBUF_SOURCE_DIR}/src"
        "${_proto_relative}"
      WORKING_DIRECTORY "${YDB_SDK_GOOGLEAPIS_SOURCE_DIR}"
      DEPENDS "${_proto}"
      VERBATIM)
    target_sources(api-common-protos PRIVATE "${_proto_cc}" "${_proto_h}")
  endforeach()

  if(YDB_SDK_INSTALL)
    install(DIRECTORY "${YDB_SDK_BINARY_DIR}/third_party/api-common-protos/google"
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
      COMPONENT libydb-cpp
      FILES_MATCHING PATTERN "*.pb.h")
  endif()
endif()

add_library(FastLZ "${YDB_SDK_FASTLZ_SOURCE_DIR}/fastlz.c")
target_include_directories(FastLZ PUBLIC
  $<BUILD_INTERFACE:${YDB_SDK_FASTLZ_SOURCE_DIR}>)

# nayuki_md5 is maintained in-tree and is not an external dependency.
add_library(nayuki_md5)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  target_sources(nayuki_md5 PRIVATE
    ${YDB_SDK_SOURCE_DIR}/third_party/nayuki_md5/nayuki_md5-fast-x8664.S)
else()
  target_sources(nayuki_md5 PRIVATE
    ${YDB_SDK_SOURCE_DIR}/third_party/nayuki_md5/nayuki_md5.c)
endif()
target_include_directories(nayuki_md5 PUBLIC
  $<BUILD_INTERFACE:${YDB_SDK_SOURCE_DIR}/third_party/nayuki_md5>
  $<INSTALL_INTERFACE:third_party/nayuki_md5>)
