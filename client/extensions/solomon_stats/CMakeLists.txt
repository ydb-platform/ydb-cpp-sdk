add_library(client-extensions-solomon_stats)

target_link_libraries(client-extensions-solomon_stats PUBLIC
  yutil
  monlib-encode-json
  cpp-monlib-metrics
  cpp-monlib-service
  monlib-service-pages
  cpp-client-ydb_extension
)

target_sources(client-extensions-solomon_stats PRIVATE
  ${CMAKE_SOURCE_DIR}/client/extensions/solomon_stats/pull_client.cpp
  ${CMAKE_SOURCE_DIR}/client/extensions/solomon_stats/pull_connector.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-extensions-solomon_stats PUBLIC
    
  )
endif()
