add_library(cpp-client-ydb_datastreams)

target_link_libraries(cpp-client-ydb_datastreams PUBLIC
  contrib-libs-linux-headers
  contrib-libs-cxxsupp
  yutil
  library-grpc-client
  cpp-string_utils-url
  api-grpc-draft
  public-lib-operation_id
  impl-ydb_internal-make_request
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_datastreams PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_datastreams/datastreams.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_datastreams PUBLIC
    contrib-libs-linux-headers
  )
endif()
