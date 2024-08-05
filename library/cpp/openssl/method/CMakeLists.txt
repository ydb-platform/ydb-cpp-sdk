_ydb_sdk_add_library(openssl-method)

target_link_libraries(openssl-method
  PUBLIC
    openssl-holders
  PRIVATE
    OpenSSL::SSL
)

target_sources(openssl-method PRIVATE
  io.cpp
)

_ydb_sdk_install_targets(TARGETS openssl-method)
