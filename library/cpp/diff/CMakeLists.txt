add_library(diff)

target_link_libraries(diff
  PUBLIC
    yutil
    lcs
)

target_sources(diff PRIVATE
  diff.cpp
)

_ydb_sdk_install_targets(TARGETS diff)
