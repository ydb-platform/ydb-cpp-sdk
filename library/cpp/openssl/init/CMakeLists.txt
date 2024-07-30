add_library(openssl-init)

target_link_libraries(openssl-init PRIVATE
  OpenSSL::SSL
)

target_sources(openssl-init PRIVATE
  init.cpp
)

_ydb_sdk_install_targets(TARGETS openssl-init)
