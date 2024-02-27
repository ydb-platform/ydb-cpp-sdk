# add_subdirectory(ut)

add_library(cpp-client-ydb_params)

target_link_libraries(cpp-client-ydb_params PUBLIC
  contrib-libs-cxxsupp
  yutil
  api-protos
  client-ydb_types-fatal_error_handlers
  cpp-client-ydb_value
  cpp-string_builder
)

target_sources(cpp-client-ydb_params PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_params/params.cpp
  ${CMAKE_SOURCE_DIR}/client/ydb_params/impl.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-client-ydb_params PUBLIC
    contrib-libs-linux-headers
  )
endif()
