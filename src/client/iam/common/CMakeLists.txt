add_library(client-iam-common)

target_link_libraries(client-iam-common PUBLIC
  yutil
  library-grpc-client
  cpp-http-simple
  library-cpp-json
  cpp-threading-atomic
  library-jwt
  client-ydb_types-credentials
  client-yc_public-iam
)

target_sources(client-iam-common PRIVATE
  iam.cpp
)

add_library(YDB-CPP-SDK::Iam ALIAS client-iam-common)
