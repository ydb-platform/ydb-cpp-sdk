add_library(impl-ydb_internal-common)

target_link_libraries(impl-ydb_internal-common PUBLIC
  yutil
  library-grpc-client
  yql-public-issue
)

target_sources(impl-ydb_internal-common PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/common/parser.cpp
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/common/getenv.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(impl-ydb_internal-common PUBLIC
    
  )
endif()
