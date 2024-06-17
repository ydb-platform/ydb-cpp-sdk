add_library(client-iam-common)

target_link_libraries(client-iam-common PUBLIC
  yutil
  library-grpc-client
  library-http-simple
  library-json
  library-jwt
  client-ydb_types-credentials
  api-client-yc_public
)

target_sources(client-iam-common PRIVATE
  iam.cpp
)

_ydb_sdk_install_targets(TARGETS client-iam-common)

add_library(YDB-CPP-SDK::Iam ALIAS client-iam-common)
