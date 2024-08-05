_ydb_sdk_add_library(monlib-metrics)

target_link_libraries(monlib-metrics
  PUBLIC
    yutil
    threading-light_rw_lock
  PRIVATE
    enum_serialization_runtime
    string_utils-misc
)

target_sources(monlib-metrics
  PRIVATE
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

generate_enum_serilization(monlib-metrics
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/metrics/metric_value_type.h
  GEN_HEADER
  ${YDB_SDK_BINARY_DIR}/library/cpp/monlib/metrics/metric_value_type.h_serialized.h
  INCLUDE_HEADERS
  library/cpp/monlib/metrics/metric_value_type.h
)

_ydb_sdk_install_targets(TARGETS monlib-metrics)
