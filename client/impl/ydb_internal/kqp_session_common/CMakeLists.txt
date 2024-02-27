add_library(impl-ydb_internal-kqp_session_common)

target_link_libraries(impl-ydb_internal-kqp_session_common PUBLIC
  contrib-libs-cxxsupp
  yutil
  cpp-threading-future
  lib-operation_id-protos
  client-impl-ydb_endpoints
)

target_sources(impl-ydb_internal-kqp_session_common PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_internal/kqp_session_common/kqp_session_common.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(impl-ydb_internal-kqp_session_common PUBLIC
    contrib-libs-linux-headers
  )
endif()
