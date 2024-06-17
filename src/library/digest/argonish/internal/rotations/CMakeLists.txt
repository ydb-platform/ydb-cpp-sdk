add_library(argonish-internal-rotations INTERFACE)

target_link_libraries(argonish-internal-rotations INTERFACE
  yutil
)

_ydb_sdk_install_targets(TARGETS argonish-internal-rotations)
