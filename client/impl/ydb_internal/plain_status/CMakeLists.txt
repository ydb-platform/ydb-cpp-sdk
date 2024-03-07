add_library(impl-ydb_internal-plain_status)

target_link_libraries(impl-ydb_internal-plain_status PUBLIC
  contrib-libs-cxxsupp
  yutil
  protobuf::libprotobuf
  library-grpc-client
  yql-public-issue
)

target_sources(impl-ydb_internal-plain_status PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/plain_status/status.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(impl-ydb_internal-plain_status PUBLIC
    contrib-libs-linux-headers
  )
endif()
