add_library(monlib-encode-prometheus)

target_link_libraries(monlib-encode-prometheus PUBLIC
  yutil
  monlib-encode
  monlib-encode-buffered
)

target_sources(monlib-encode-prometheus PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/prometheus/prometheus_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/prometheus/prometheus_encoder.cpp
)

_ydb_sdk_install_targets(TARGETS monlib-encode-prometheus)
