add_library(impl-ydb_internal-plain_status)

target_link_libraries(impl-ydb_internal-plain_status PUBLIC
  yutil
  protobuf::libprotobuf
  library-grpc-client
  yql-public-issue
)

target_sources(impl-ydb_internal-plain_status PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/plain_status/status.cpp
)


