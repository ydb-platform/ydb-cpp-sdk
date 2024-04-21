add_library(client-ydb_types-operation)

target_link_libraries(client-ydb_types-operation PUBLIC
  yutil
  protobuf::libprotobuf
  cpp-threading-future
  library-operation_id
  cpp-client-ydb_types
)

target_sources(client-ydb_types-operation PRIVATE
  operation.cpp
)


