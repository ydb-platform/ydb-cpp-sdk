add_subdirectory(codecs)
add_subdirectory(impl)
add_subdirectory(include)

_ydb_sdk_add_library(cpp-client-ydb_persqueue_public INTERFACE)

target_link_libraries(cpp-client-ydb_persqueue_public INTERFACE
  yutil
  client-ydb_persqueue_public-include
  client-ydb_persqueue_public-impl
  client-ydb_persqueue_public-codecs
)
