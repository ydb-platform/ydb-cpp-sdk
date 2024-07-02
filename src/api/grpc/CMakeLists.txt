add_subdirectory(draft)

file(GLOB API_GRPC_SOURCES
  *.proto
)

_ydb_sdk_add_proto_library(api-grpc GRPC
  SOURCES ${API_GRPC_SOURCES}
)

target_include_directories(api-grpc PUBLIC
  $<BUILD_INTERFACE:${YDB_SDK_BINARY_DIR}/ydb-cpp-sdk>
)

target_link_libraries(api-grpc PUBLIC
  api-protos
)

_ydb_sdk_install_targets(TARGETS api-grpc)
