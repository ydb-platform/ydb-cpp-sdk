add_library(impl-ydb_internal-kqp_session_common)

target_link_libraries(impl-ydb_internal-kqp_session_common PUBLIC
  yutil
  cpp-threading-future
  lib-operation_id-protos
  client-impl-ydb_endpoints
)

target_sources(impl-ydb_internal-kqp_session_common PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/kqp_session_common/kqp_session_common.cpp
)


