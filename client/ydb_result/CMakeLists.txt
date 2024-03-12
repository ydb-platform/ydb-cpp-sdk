# add_subdirectory(ut)

add_library(cpp-client-ydb_result)

target_link_libraries(cpp-client-ydb_result PUBLIC
  yutil
  api-protos
  client-ydb_types-fatal_error_handlers
  cpp-client-ydb_value
  cpp-client-ydb_proto
  )
target_sources(cpp-client-ydb_result PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_result/proto_accessor.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_result/result.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_result PUBLIC
    
  )
endif()
