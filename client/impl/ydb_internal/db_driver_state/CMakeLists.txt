add_library(impl-ydb_internal-db_driver_state)

target_link_libraries(impl-ydb_internal-db_driver_state PUBLIC
  yutil
  cpp-string_utils-quote
  cpp-threading-future
  client-impl-ydb_endpoints
  impl-ydb_internal-logger
  impl-ydb_internal-plain_status
  client-ydb_types-credentials
)

target_sources(impl-ydb_internal-db_driver_state PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/db_driver_state/authenticator.cpp
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/db_driver_state/endpoint_pool.cpp
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/db_driver_state/state.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
target_link_libraries(impl-ydb_internal-db_driver_state PUBLIC
  
)
endif()
