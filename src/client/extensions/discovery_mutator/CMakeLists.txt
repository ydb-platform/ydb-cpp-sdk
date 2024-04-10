add_library(client-extensions-discovery_mutator)

target_link_libraries(client-extensions-discovery_mutator PUBLIC
  yutil
  cpp-client-ydb_extension
)

target_sources(client-extensions-discovery_mutator PRIVATE
<<<<<<< HEAD:src/client/extensions/discovery_mutator/CMakeLists.txt
  discovery_mutator.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/extensions/discovery_mutator/discovery_mutator.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/extensions/discovery_mutator/CMakeLists.txt
)

add_library(YDB-CPP-SDK::DiscoveryMutator ALIAS client-extensions-discovery_mutator)
