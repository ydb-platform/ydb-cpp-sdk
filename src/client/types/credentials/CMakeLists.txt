add_subdirectory(login)
add_subdirectory(oauth2_token_exchange)

_ydb_sdk_add_library(client-types-credentials)

target_link_libraries(client-types-credentials PUBLIC
  yutil
  api-grpc
  client-types-status
  yql-public-issue
  client-types-credentials-login
  client-types-credentials-oauth2
)

target_sources(client-types-credentials PRIVATE
  credentials.cpp
)

_ydb_sdk_make_client_component(Credentials client-types-credentials)
