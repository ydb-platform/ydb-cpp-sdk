add_library(internal-proxies-macro INTERFACE)

target_link_libraries(internal-proxies-macro INTERFACE
  yutil
)

_ydb_sdk_install_targets(TARGETS internal-proxies-macro)
