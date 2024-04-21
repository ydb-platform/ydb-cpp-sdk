add_library(impl-ydb_internal-common)

target_link_libraries(impl-ydb_internal-common PUBLIC
  yutil
  library-grpc-client
  yql-public-issue
)

target_sources(impl-ydb_internal-common PRIVATE
  parser.cpp
  getenv.cpp
)


