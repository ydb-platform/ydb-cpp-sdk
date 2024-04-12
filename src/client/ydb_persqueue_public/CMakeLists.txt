add_subdirectory(codecs)

add_library(cpp-client-ydb_persqueue_public INTERFACE)

target_link_libraries(cpp-client-ydb_persqueue_public INTERFACE
  yutil
  cpp-client-ydb_persqueue_core
  client-ydb_persqueue_public-codecs
)
