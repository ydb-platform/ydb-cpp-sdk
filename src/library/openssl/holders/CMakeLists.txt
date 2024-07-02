add_library(openssl-holders)

target_link_libraries(openssl-holders PUBLIC
  yutil
  OpenSSL::SSL
)

target_sources(openssl-holders PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/holders/bio.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/holders/x509_vfy.cpp
)

_ydb_sdk_install_targets(TARGETS openssl-holders)
