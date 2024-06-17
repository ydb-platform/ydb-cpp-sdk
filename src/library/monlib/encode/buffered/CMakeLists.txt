add_library(monlib-encode-buffered)

target_link_libraries(monlib-encode-buffered PUBLIC
  yutil
  monlib-encode
  monlib-metrics
)

target_sources(monlib-encode-buffered PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/buffered_encoder_base.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/string_pool.cpp
)

_ydb_sdk_install_targets(TARGETS monlib-encode-buffered)
