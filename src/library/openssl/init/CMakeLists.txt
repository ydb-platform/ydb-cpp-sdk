add_library(openssl-init)

target_link_libraries(openssl-init PUBLIC
  yutil
  OpenSSL::SSL
)

target_sources(openssl-init PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/init/init.cpp
)

_ydb_sdk_install_targets(TARGETS openssl-init)
