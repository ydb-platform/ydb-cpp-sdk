_ydb_sdk_add_library(openssl-io)

target_link_libraries(openssl-io
  PUBLIC
    yutil
  PRIVATE
    OpenSSL::SSL
    openssl-init
    openssl-method
    resource
)

target_sources(openssl-io PRIVATE
  stream.cpp
)

_ydb_sdk_install_targets(TARGETS openssl-io)
