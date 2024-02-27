add_subdirectory(login)

add_library(client-ydb_types-credentials)

target_link_libraries(client-ydb_types-credentials PUBLIC
  contrib-libs-cxxsupp
  yutil
  ydb-library-login
  api-grpc
  client-ydb_types-status
  yql-public-issue
)

target_sources(client-ydb_types-credentials PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_types/credentials/credentials.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-ydb_types-credentials PUBLIC
    contrib-libs-linux-headers
  )
endif()
