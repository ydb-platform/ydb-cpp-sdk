add_library(client-extensions-discovery_mutator)

target_link_libraries(client-extensions-discovery_mutator PUBLIC
  yutil
  client-ydb_extension
)

target_sources(client-extensions-discovery_mutator PRIVATE
  discovery_mutator.cpp
)

_ydb_sdk_make_client_component(DiscoveryMutator client-extensions-discovery_mutator)
