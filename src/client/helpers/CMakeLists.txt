_ydb_sdk_add_library(client-helpers)

target_link_libraries(client-helpers
  PUBLIC
    yutil
    client-types-credentials-oauth2
    client-iam
    client-types-credentials
)

target_sources(client-helpers
  PRIVATE
    helpers.cpp
)

_ydb_sdk_make_client_component(Helpers client-helpers)
