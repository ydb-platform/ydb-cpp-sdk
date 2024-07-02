add_library(library-grpc-client)

target_link_libraries(library-grpc-client PUBLIC
  yutil
  gRPC::grpc++
  deprecated-atomic
)

target_sources(library-grpc-client PRIVATE
  grpc_client_low.cpp
)

_ydb_sdk_install_targets(TARGETS library-grpc-client)
