_ydb_sdk_add_library(impl-internal-sdk_runtime)

target_link_libraries(impl-internal-sdk_runtime PUBLIC
  grpc-client
)

target_sources(impl-internal-sdk_runtime PRIVATE
  runtime.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-sdk_runtime)
