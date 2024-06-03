add_library(client-ydb_persqueue_core-impl)

target_link_libraries(client-ydb_persqueue_core-impl PUBLIC
  yutil
  client-ydb_topic-impl
)

target_sources(client-ydb_persqueue_core-impl PRIVATE
  common.cpp
)
