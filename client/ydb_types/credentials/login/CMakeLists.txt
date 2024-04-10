add_library(ydb_types-credentials-login)

target_link_libraries(ydb_types-credentials-login PUBLIC
  yutil
  ydb-library-login
  ydb-library-security
  api-grpc
  client-ydb_types-status
  impl-ydb_internal-grpc_connections
  yql-public-issue
)

target_sources(ydb_types-credentials-login PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/ydb_types/credentials/login/login.cpp
)


