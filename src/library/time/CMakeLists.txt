_ydb_sdk_add_library(ydb-time)

target_link_libraries(ydb-time
  PUBLIC
    yutil
)

target_sources(ydb-time PRIVATE
  time.cpp
)

_ydb_sdk_install_targets(TARGETS ydb-time)
