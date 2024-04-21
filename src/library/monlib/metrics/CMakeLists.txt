add_library(cpp-monlib-metrics)

target_link_libraries(cpp-monlib-metrics PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  cpp-string_utils-misc
)

target_sources(cpp-monlib-metrics PRIVATE
  ewma.cpp
  fake.cpp
  histogram_collector_explicit.cpp
  histogram_collector_exponential.cpp
  histogram_collector_linear.cpp
  histogram_snapshot.cpp
  log_histogram_snapshot.cpp
  labels.cpp
  metric_registry.cpp
  metric_consumer.cpp
  metric_type.cpp
  metric_value.cpp
  summary_snapshot.cpp
)

generate_enum_serilization(cpp-monlib-metrics
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/metric_value_type.h
  GEN_HEADER
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/metrics/metric_value_type.h_serialized.h
  INCLUDE_HEADERS
  src/library/monlib/metrics/metric_value_type.h
)


