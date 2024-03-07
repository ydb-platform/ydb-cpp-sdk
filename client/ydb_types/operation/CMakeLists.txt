add_library(client-ydb_types-operation)

target_link_libraries(client-ydb_types-operation PUBLIC
  contrib-libs-cxxsupp
  yutil
  protobuf::libprotobuf
  cpp-threading-future
  public-lib-operation_id
  cpp-client-ydb_types
  )

target_sources(client-ydb_types-operation PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_types/operation/operation.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-ydb_types-operation PUBLIC
    contrib-libs-linux-headers
  )
endif()
