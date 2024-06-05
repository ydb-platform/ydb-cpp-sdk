add_library(cpp-client-ss_tasks)

target_link_libraries(cpp-client-ss_tasks PUBLIC
  yutil
  api-grpc
  api-protos
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-client-ydb_proto
  client-ydb_types-operation
)

target_sources(cpp-client-ss_tasks PRIVATE
  task.cpp
  out.cpp
)

generate_enum_serilization(cpp-client-ss_tasks
  ${YDB_SDK_SOURCE_DIR}/src/client/ss_tasks/task.h
  INCLUDE_HEADERS
  src/client/ss_tasks/task.h
)
