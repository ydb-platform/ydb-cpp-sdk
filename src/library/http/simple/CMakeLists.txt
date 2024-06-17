add_library(library-http-simple)

target_link_libraries(library-http-simple PUBLIC
  yutil
  library-http-io
  openssl-io
  string_utils-url
  library-uri
)

target_sources(library-http-simple PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/http/simple/http_client.cpp
)

_ydb_sdk_install_targets(TARGETS library-http-simple)
