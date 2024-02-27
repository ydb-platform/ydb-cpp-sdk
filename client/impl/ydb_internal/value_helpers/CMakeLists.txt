add_library(impl-ydb_internal-value_helpers)

target_link_libraries(impl-ydb_internal-value_helpers PUBLIC
  contrib-libs-cxxsupp
  yutil
  api-protos
  client-ydb_types-fatal_error_handlers
  cpp-string_builder
)

target_sources(impl-ydb_internal-value_helpers PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/value_helpers/helpers.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(impl-ydb_internal-value_helpers PUBLIC
    contrib-libs-linux-headers
  )
endif()
