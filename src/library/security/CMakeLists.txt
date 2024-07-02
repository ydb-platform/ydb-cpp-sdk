add_library(library-security INTERFACE)

target_link_libraries(library-security INTERFACE
  yutil
)

_ydb_sdk_install_targets(TARGETS library-security)
