_ydb_sdk_add_library(openssl-crypto)

target_sources(openssl-crypto PRIVATE sha.cpp)
target_link_libraries(openssl-crypto PUBLIC yutil OpenSSL::Crypto)

_ydb_sdk_install_targets(TARGETS openssl-crypto)
