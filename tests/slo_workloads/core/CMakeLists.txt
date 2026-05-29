include(FetchContent)

FetchContent_Declare(
  hdr_histogram
  GIT_REPOSITORY https://github.com/HdrHistogram/HdrHistogram_c.git
  GIT_TAG        0.11.8
  EXCLUDE_FROM_ALL
)
set(HDR_HISTOGRAM_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(HDR_HISTOGRAM_BUILD_SHARED   OFF CACHE BOOL "" FORCE)
set(HDR_LOG_REQUIRED             OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(hdr_histogram)

add_library(slo-workload-core)

target_link_libraries(slo-workload-core PUBLIC
  yutil
  client-types-status
  client-ydb_driver
  opentelemetry-cpp::metrics
  opentelemetry-cpp::otlp_http_metric_exporter
  hdr_histogram_static
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
