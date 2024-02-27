add_library(client-impl-ydb_stats)

target_link_libraries(client-impl-ydb_stats PUBLIC
  contrib-libs-cxxsupp
  yutil
  library-grpc-client
  cpp-monlib-metrics
  cpp-string_builder
)

target_sources(client-impl-ydb_stats PRIVATE
  ${CMAKE_SOURCE_DIR}/client/impl/ydb_stats/stats.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-impl-ydb_stats PUBLIC
    contrib-libs-linux-headers
  )
endif()
