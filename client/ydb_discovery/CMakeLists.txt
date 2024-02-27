add_library(cpp-client-ydb_discovery)

target_link_libraries(cpp-client-ydb_discovery PUBLIC
  contrib-libs-cxxsupp
  yutil
  client-ydb_common_client-impl
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_discovery PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_discovery/discovery.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
target_link_libraries(cpp-client-ydb_discovery PUBLIC
  contrib-libs-linux-headers
)
endif()
