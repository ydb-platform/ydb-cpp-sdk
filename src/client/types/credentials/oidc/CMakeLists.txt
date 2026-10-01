_ydb_sdk_add_library(client-types-credentials-oidc)

target_sources(client-types-credentials-oidc PRIVATE
  client_provider.cpp
  credentials.cpp
  device_provider.cpp
  private.cpp
  protocol.cpp
  provider_base.cpp
  static_provider.cpp
)

target_link_libraries(client-types-credentials-oidc PUBLIC
  yutil
  http-simple
  json
  openssl-crypto
  string_utils-base64
  string_utils-quote
  client-types
  client-types-credentials
)

_ydb_sdk_install_targets(TARGETS client-types-credentials-oidc)
