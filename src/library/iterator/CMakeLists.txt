add_library(library-iterator INTERFACE)

target_link_libraries(library-iterator INTERFACE
  yutil
)

_ydb_sdk_install_targets(TARGETS library-iterator)
