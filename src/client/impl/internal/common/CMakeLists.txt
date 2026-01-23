_ydb_sdk_add_library(impl-internal-common)

target_link_libraries(impl-internal-common PUBLIC
  yutil
  cgiparam
  uri
  client-types-exceptions
  grpc-client
  yql-public-issue
)

target_sources(impl-internal-common PRIVATE
  balancing_policies.cpp
  parser.cpp
  getenv.cpp
  client_pid.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-common)
