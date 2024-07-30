add_library(http-simple)

target_link_libraries(http-simple
  PUBLIC
    yutil
    http-io
    http-misc
    openssl-io
    string_utils-url
  PRIVATE
    library-uri
)

target_sources(http-simple PRIVATE
  http_client.cpp
)

_ydb_sdk_install_targets(TARGETS http-simple)
