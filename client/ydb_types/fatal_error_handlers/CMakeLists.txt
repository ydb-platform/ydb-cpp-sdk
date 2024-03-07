add_library(client-ydb_types-fatal_error_handlers)

target_link_libraries(client-ydb_types-fatal_error_handlers PUBLIC
  yutil
  client-ydb_types-exceptions
)

target_sources(client-ydb_types-fatal_error_handlers PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_types/fatal_error_handlers/handlers.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-ydb_types-fatal_error_handlers PUBLIC
    
  )
endif()
