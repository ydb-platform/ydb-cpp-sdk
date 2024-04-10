add_library(monlib-encode-prometheus)

target_link_libraries(monlib-encode-prometheus PUBLIC
  yutil
  cpp-monlib-encode
  monlib-encode-buffered
)

target_sources(monlib-encode-prometheus PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/prometheus/prometheus_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/prometheus/prometheus_encoder.cpp
)


