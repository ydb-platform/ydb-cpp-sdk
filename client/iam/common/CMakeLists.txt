add_library(client-iam-common)

target_link_libraries(client-iam-common PUBLIC
  yutil
  library-grpc-client
  cpp-http-simple
  library-cpp-json
  cpp-threading-atomic
  public-lib-jwt
  client-ydb_types-credentials
)

target_sources(client-iam-common PRIVATE
  ${CMAKE_SOURCE_DIR}/client/iam/common/iam.cpp
)


