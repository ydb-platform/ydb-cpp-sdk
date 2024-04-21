add_subdirectory(impl)
# add_subdirectory(ut)

add_library(cpp-client-ydb_persqueue_core)

target_link_libraries(cpp-client-ydb_persqueue_core PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  library-cpp-retry
  client-ydb_persqueue_core-impl
  cpp-client-ydb_proto
  cpp-client-ydb_driver
  api-grpc
  api-grpc-draft
  api-protos
)

generate_enum_serilization(cpp-client-ydb_persqueue_core
  ${YDB_SDK_SOURCE_DIR}/src/client/persqueue_core/persqueue.h
  INCLUDE_HEADERS
  src/client/persqueue_core/persqueue.h
)
