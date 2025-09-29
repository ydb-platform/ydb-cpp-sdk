_ydb_sdk_add_library(impl-internal-retry)

target_link_libraries(impl-internal-retry PUBLIC
  yutil
  impl-internal-grpc_connections
)

target_sources(impl-internal-retry PRIVATE
  retry.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-retry)
