add_library(ydb_types-credentials-oauth2)

target_link_libraries(ydb_types-credentials-oauth2 PUBLIC
  yutil
  jwt-cpp::jwt-cpp
  library-cpp-cgiparam
  cpp-http-misc
  cpp-http-simple
  library-cpp-json
  library-cpp-retry
  library-cpp-uri
  client-ydb_types-credentials
  cpp-client-ydb_types
)

target_sources(ydb_types-credentials-oauth2 PRIVATE
  credentials.cpp
  jwt_token_source.cpp
)
