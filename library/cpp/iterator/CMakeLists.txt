_ydb_sdk_add_library(iterator)

target_link_libraries(iterator PUBLIC
  yutil
)

target_sources(iterator PRIVATE
  cartesian_product.cpp
  concatenate.cpp
  enumerate.cpp
  filtering.cpp
  functools.cpp
  iterate_keys.cpp
  iterate_values.cpp
  mapped.cpp
  zip.cpp
)

_ydb_sdk_install_targets(TARGETS iterator)
