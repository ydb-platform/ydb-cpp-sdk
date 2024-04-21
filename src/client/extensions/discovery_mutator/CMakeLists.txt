add_library(client-extensions-discovery_mutator)

target_link_libraries(client-extensions-discovery_mutator PUBLIC
  yutil
  cpp-client-ydb_extension
)

target_sources(client-extensions-discovery_mutator PRIVATE
  discovery_mutator.cpp
)

add_library(YDB-CPP-SDK::DiscoveryMutator ALIAS client-extensions-discovery_mutator)
