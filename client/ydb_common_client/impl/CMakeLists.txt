add_library(client-ydb_common_client-impl)

target_link_libraries(client-ydb_common_client-impl PUBLIC
  yutil
  impl-ydb_internal-grpc_connections
)

target_sources(client-ydb_common_client-impl PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_common_client/impl/client.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-ydb_common_client-impl PUBLIC
    
  )
endif()
