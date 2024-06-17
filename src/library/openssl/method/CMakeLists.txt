add_library(openssl-method)

target_link_libraries(openssl-method PUBLIC
  yutil
  OpenSSL::SSL
  openssl-holders
)

target_sources(openssl-method PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/method/io.cpp
)

_ydb_sdk_install_targets(TARGETS openssl-method)
