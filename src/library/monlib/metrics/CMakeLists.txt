add_library(cpp-monlib-metrics)

target_link_libraries(cpp-monlib-metrics PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  cpp-string_utils-misc
)

target_sources(cpp-monlib-metrics PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/ewma.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/fake.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/histogram_collector_explicit.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/histogram_collector_exponential.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/histogram_collector_linear.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/histogram_snapshot.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/log_histogram_snapshot.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/labels.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/metric_registry.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/metric_consumer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/metric_type.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/metric_value.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/summary_collector.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/summary_snapshot.cpp
)

generate_enum_serilization(cpp-monlib-metrics
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/metrics/metric_value_type.h
  GEN_HEADER
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/metrics/metric_value_type.h_serialized.h
  INCLUDE_HEADERS
  src/library/monlib/metrics/metric_value_type.h
)


