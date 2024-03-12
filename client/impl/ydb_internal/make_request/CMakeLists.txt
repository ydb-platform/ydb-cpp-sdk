add_library(impl-ydb_internal-make_request)

target_link_libraries(impl-ydb_internal-make_request PUBLIC
  yutil
  protobuf::libprotobuf
  api-protos
)

target_sources(impl-ydb_internal-make_request PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/make_request/make.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(impl-ydb_internal-make_request PUBLIC
    
  )
endif()
