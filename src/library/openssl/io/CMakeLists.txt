add_library(openssl-io)

target_link_libraries(openssl-io PUBLIC
  yutil
  OpenSSL::SSL
  openssl-init
  openssl-method
)

target_sources(openssl-io PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/io/stream.cpp
)

_ydb_sdk_install_targets(TARGETS openssl-io)
