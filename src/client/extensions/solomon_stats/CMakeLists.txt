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
<<<<<<< HEAD:src/client/extensions/solomon_stats/CMakeLists.txt
  pull_client.cpp
  pull_connector.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/extensions/solomon_stats/pull_client.cpp
  ${YDB_SDK_SOURCE_DIR}/client/extensions/solomon_stats/pull_connector.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/extensions/solomon_stats/CMakeLists.txt
)

add_library(YDB-CPP-SDK::SolomonStats ALIAS client-extensions-solomon_stats)
