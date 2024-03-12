add_library(impl-ydb_internal-retry)

target_link_libraries(impl-ydb_internal-retry PUBLIC
  yutil
  impl-ydb_internal-grpc_connections
)

target_sources(impl-ydb_internal-retry PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/retry/retry.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(impl-ydb_internal-retry PUBLIC
    
  )
endif()
