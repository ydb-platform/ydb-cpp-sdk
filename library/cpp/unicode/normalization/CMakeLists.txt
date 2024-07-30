add_library(unicode-normalization)

target_link_libraries(unicode-normalization PUBLIC
  yutil
)

target_sources(unicode-normalization
  PRIVATE
    generated/composition.cpp
    generated/decomposition.cpp
    normalization.cpp
)

_ydb_sdk_install_targets(TARGETS unicode-normalization)
