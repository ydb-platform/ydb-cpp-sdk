add_library(ydb_types-credentials-login)

target_link_libraries(ydb_types-credentials-login PUBLIC
  yutil
  library-login
  library-security
  api-grpc
  client-ydb_types-status
  impl-ydb_internal-grpc_connections
  yql-public-issue
)

target_sources(ydb_types-credentials-login PRIVATE
  login.cpp
)


