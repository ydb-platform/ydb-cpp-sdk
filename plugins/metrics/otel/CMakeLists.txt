_ydb_sdk_add_library(open_telemetry_metrics)

target_sources(open_telemetry_metrics PRIVATE
  src/metrics.cpp
)
target_include_directories(open_telemetry_metrics PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(open_telemetry_metrics PUBLIC
  client-metrics
  client-resources
  opentelemetry-cpp::api
  opentelemetry-cpp::metrics
)
_ydb_sdk_make_client_component(OpenTelemetryMetrics open_telemetry_metrics)

_ydb_sdk_install_headers(${CMAKE_INSTALL_INCLUDEDIR} DIRECTORY include/)