_ydb_sdk_add_library(tests-iam-mocks)

target_link_libraries(tests-iam-mocks PUBLIC
  yutil
  http-server
  http-misc
  json
  cpp-testing-common
  api-client-yc_public
)

target_sources(tests-iam-mocks PRIVATE
  iam_http_mock_server.cpp
  iam_grpc_mock_server.cpp
)
