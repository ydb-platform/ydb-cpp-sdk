_ydb_sdk_add_library(client-types-operation)

target_link_libraries(client-types-operation PUBLIC
  yutil
  protobuf::libprotobuf
  threading-future
  library-operation_id
  client-types
)

target_sources(client-types-operation PRIVATE
  operation.cpp
  out.cpp
)

_ydb_sdk_install_targets(TARGETS client-types-operation)
