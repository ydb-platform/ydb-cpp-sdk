add_library(impl-ydb_internal-logger)

target_link_libraries(impl-ydb_internal-logger PUBLIC
  yutil
  library-cpp-logger
)

target_sources(impl-ydb_internal-logger PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/logger/log.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(impl-ydb_internal-logger PUBLIC
    
  )
endif()
