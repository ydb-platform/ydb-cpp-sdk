add_library(unicode_normalization)

target_link_libraries(unicode_normalization PUBLIC
  yutil
)

target_sources(unicode_normalization PRIVATE
  generated/composition.cpp
  generated/decomposition.cpp
  normalization.cpp
)

_ydb_sdk_install_targets(TARGETS unicode_normalization)
