_ydb_sdk_add_library(client-oauth2-ut-helpers)

target_sources(client-oauth2-ut-helpers
  PRIVATE
    test_token_exchange_server.cpp
)

target_link_libraries(client-oauth2-ut-helpers
  PUBLIC
    jwt-cpp::jwt-cpp
    cgiparam
    http-misc
    http-server
    json
    string_utils-base64
    client-types-credentials-oauth2
)
