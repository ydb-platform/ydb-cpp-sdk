add_library(ydb_types-credentials-login)

target_link_libraries(ydb_types-credentials-login PUBLIC
  contrib-libs-cxxsupp
  yutil
  ydb-library-login
  ydb-library-security
  api-grpc
  client-ydb_types-status
  impl-ydb_internal-grpc_connections
  yql-public-issue
)

target_sources(ydb_types-credentials-login PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_types/credentials/login/login.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(ydb_types-credentials-login PUBLIC
    contrib-libs-linux-headers
  )
endif()
