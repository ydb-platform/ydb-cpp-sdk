add_subdirectory(impl)
# add_subdirectory(ut)

add_library(cpp-client-ydb_persqueue_core INTERFACE)

target_link_libraries(cpp-client-ydb_persqueue_core INTERFACE
  cpp-client-ydb_persqueue_public
  client-ydb_persqueue_core-impl
)
