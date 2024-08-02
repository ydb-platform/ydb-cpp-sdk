_ydb_sdk_add_library(openssl-holders)

target_link_libraries(openssl-holders
  PUBLIC
    yutil
    OpenSSL::SSL
)

target_sources(openssl-holders
  PRIVATE
    bio.cpp
    x509_vfy.cpp
)

_ydb_sdk_install_targets(TARGETS openssl-holders)
