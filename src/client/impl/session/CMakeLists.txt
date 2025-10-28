_ydb_sdk_add_library(impl-session)

target_link_libraries(impl-session PUBLIC
  yutil
  threading-future
  api-protos
  client-impl-ydb_endpoints
  client-types-operation  
)

target_sources(impl-session PRIVATE
  kqp_session_common.cpp
  session_pool.cpp
)

_ydb_sdk_install_targets(TARGETS impl-session)
