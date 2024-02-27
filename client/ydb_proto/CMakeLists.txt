add_library(cpp-client-ydb_proto)

target_link_libraries(cpp-client-ydb_proto PUBLIC
  contrib-libs-cxxsupp
  yutil
  api-grpc
  api-grpc-draft
  api-protos
  lib-operation_id-protos
  cpp-client-ydb_params
  cpp-client-ydb_value
  public-issue-protos
)

target_sources(cpp-client-ydb_proto PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_proto/accessor.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_proto PUBLIC
    contrib-libs-linux-headers
  )
endif()
