add_library(impl-ydb_internal-logger)

target_link_libraries(impl-ydb_internal-logger PUBLIC
  contrib-libs-cxxsupp
  yutil
  library-cpp-logger
  cpp-string_builder
)

target_sources(impl-ydb_internal-logger PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/logger/log.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(impl-ydb_internal-logger PUBLIC
    contrib-libs-linux-headers
  )
endif()
