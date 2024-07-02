add_ydb_test(NAME client-oauth2-ut
  SOURCES
  credentials_ut.cpp
  jwt_token_source_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  library-http-server
  client-ydb_types-credentials-oauth2
)
