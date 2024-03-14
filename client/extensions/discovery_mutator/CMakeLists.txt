# add_subdirectory(ut)

add_library(client-extensions-discovery_mutator)

target_link_libraries(client-extensions-discovery_mutator PUBLIC
  yutil
  cpp-client-ydb_extension
)

target_sources(client-extensions-discovery_mutator PRIVATE
  ${CMAKE_SOURCE_DIR}/client/extensions/discovery_mutator/discovery_mutator.cpp
)


