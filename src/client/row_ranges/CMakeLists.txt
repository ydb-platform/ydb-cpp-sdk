_ydb_sdk_add_library(client-ydb_row_ranges)

target_link_libraries(client-ydb_row_ranges PUBLIC
  yutil
  threading-future
  client-ydb_result
  client-ydb_query
  client-ydb_table
)

target_sources(client-ydb_row_ranges PRIVATE
  rows.cpp
)

_ydb_sdk_install_targets(TARGETS client-ydb_row_ranges)
