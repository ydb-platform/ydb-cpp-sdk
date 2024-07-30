file(GLOB_RECURSE API_PROTOS_SOURCES
  *.proto
)

_ydb_sdk_add_proto_library(api-protos
  SOURCES ${API_PROTOS_SOURCES}
  OUT_DIR ydb-cpp-sdk
  PUBLIC_PROTOS 
    ydb_federation_discovery
    ydb_value
    ydb_query
    ydb_topic
    ydb_table
    ydb_query_stats
    ydb_import
    ydb_export
    ydb_coordination
    ydb_replication
)

target_compile_options(api-protos
  PRIVATE
    -Wno-deprecated
)

target_link_libraries(api-protos PRIVATE
  enum_serialization_runtime
)

target_include_directories(api-protos PUBLIC
  $<BUILD_INTERFACE:${YDB_SDK_BINARY_DIR}/ydb-cpp-sdk>
)

_ydb_sdk_install_targets(TARGETS api-protos)
