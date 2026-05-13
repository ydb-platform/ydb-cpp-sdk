add_library(slo-workload-core)

target_link_libraries(slo-workload-core PUBLIC
  yutil
  opentelemetry-cpp::metrics
  opentelemetry-cpp::otlp_http_metric_exporter
)

target_sources(slo-workload-core PRIVATE
  duration_meter.cpp
  rps.cpp
  slo_text_utils.cpp
  shard.cpp
  metrics.cpp
  statistics.cpp
  generator.cpp
)
