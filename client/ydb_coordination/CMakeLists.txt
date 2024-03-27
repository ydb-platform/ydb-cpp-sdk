#add_subdirectory(ut)

add_library(cpp-client-ydb_coordination)

target_link_libraries(cpp-client-ydb_coordination PUBLIC
  
  yutil
  tools-enum_parser-enum_serialization_runtime
  api-grpc
  impl-ydb_internal-make_request
  cpp-client-ydb_common_client
  client-ydb_common_client-impl
  cpp-client-ydb_driver
  cpp-client-ydb_proto
  cpp-client-ydb_types
  client-ydb_types-status
)

target_sources(cpp-client-ydb_coordination PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_coordination/coordination.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_coordination/proto_accessor.cpp
)

generate_enum_serilization(cpp-client-ydb_coordination
  ${CMAKE_SOURCE_DIR}/client/ydb_coordination/coordination.h
  INCLUDE_HEADERS
  client/ydb_coordination/coordination.h
)

add_library(YDB-CPP-SDK::Coordination ALIAS cpp-client-ydb_coordination)
