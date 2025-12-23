add_library(slo-utils)

target_link_libraries(slo-utils PUBLIC
  yutil
  getopt
  YDB-CPP-SDK::Table
  YDB-CPP-SDK::Iam
  opentelemetry-cpp::metrics
  opentelemetry-cpp::otlp_http_metric_exporter
)

target_sources(slo-utils PRIVATE
  executor.cpp
  generator.cpp
  job.cpp
  metrics.cpp
  statistics.cpp
  utils.cpp
)
