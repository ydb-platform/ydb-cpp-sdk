_ydb_sdk_add_library(impl-internal-plain_status)

target_link_libraries(impl-internal-plain_status PUBLIC
  yutil
  protobuf::libprotobuf
  grpc-client
  yql-public-issue
)

target_sources(impl-internal-plain_status PRIVATE
  status.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-plain_status)
