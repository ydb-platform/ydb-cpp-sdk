file(GLOB_RECURSE API_PROTOS_SOURCES
  *.proto
)

_ydb_sdk_add_proto_library(api-protos
  SOURCES ${API_PROTOS_SOURCES}
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
