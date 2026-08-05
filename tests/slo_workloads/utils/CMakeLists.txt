# slo-utils-base: shared helpers, no DoMain. The userver workload links this
# and supplies its own DoMain that wraps command dispatch in RunStandalone.
add_library(slo-utils-base)

target_link_libraries(slo-utils-base PUBLIC
  yutil
  getopt
  YDB-CPP-SDK::Table
  YDB-CPP-SDK::Iam
  opentelemetry-cpp::metrics
  opentelemetry-cpp::otlp_http_metric_exporter
)

target_link_libraries(slo-utils-base PRIVATE
  hdr_histogram_static
)

target_sources(slo-utils-base PRIVATE
  executor.cpp
  generator.cpp
  job.cpp
  metrics.cpp
  statistics.cpp
  utils.cpp
)

# slo-utils: slo-utils-base + DoMain. Native workload links this.
# The set of .o files linked into slo-key-value is byte-equivalent to the
# pre-split single-library layout used in slo/align-with-action-v2.
add_library(slo-utils)

target_link_libraries(slo-utils PUBLIC
  slo-utils-base
)

target_sources(slo-utils PRIVATE
  utils_main.cpp
)
