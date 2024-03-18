option(USE_IDN "Search IDN library" ON)
option(USE_ICONV "Search Iconv library" ON)
option(USE_OPENSSL "Search OpenSSL library" ON)
option(USE_PROTOBUF "Search Protobuf library" ON)
option(USE_GRPC "Search gRPC library" ON)
set(YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET "" CACHE STRING "Name of cmake target preparing google common proto library")

if (USE_IDN)
  find_package(IDN REQUIRED)
endif ()

if (USE_ICONV)
  find_package(Iconv REQUIRED)
endif ()

if (USE_OPENSSL)
  find_package(OpenSSL REQUIRED)
endif ()

if (USE_PROTOBUF)
  find_package(Protobuf REQUIRED)
endif ()

if (USE_GRPC)
  find_package(gRPC REQUIRED)
endif ()

# api-common-protos
if (YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET)
  set(api-common-protos ${YDB_SDK_GOOGLE_COMMON_PROTOS_TARGET})
else ()
  add_library(api-common-protos)

  set_property(TARGET api-common-protos PROPERTY
    PROTOC_EXTRA_OUTS .grpc.pb.cc .grpc.pb.h
  )

  set_property(TARGET api-common-protos PROPERTY
    PROTO_NAMESPACE third_party/api-common-protos
  )

  target_include_directories(api-common-protos PUBLIC
    ${CMAKE_BINARY_DIR}/third_party/api-common-protos
  )

  target_link_libraries(api-common-protos PUBLIC
    gRPC::grpc++
    protobuf::libprotobuf
  )

  file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/third_party/api-common-protos/google/*.proto
  )

  target_proto_messages(api-common-protos PRIVATE
    ${SOURCES}
  )

  target_proto_addincls(api-common-protos
    ./third_party/api-common-protos
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/api-common-protos
  )

  target_proto_outs(api-common-protos
    --cpp_out=${CMAKE_BINARY_DIR}/third_party/api-common-protos
  )

  target_proto_plugin(api-common-protos
    grpc_cpp
    gRPC::grpc_cpp_plugin
  )
endif ()
