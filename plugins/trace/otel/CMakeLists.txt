_ydb_sdk_add_library(open_telemetry_trace)
target_sources(open_telemetry_trace PRIVATE
  src/trace.cpp
)
target_include_directories(open_telemetry_trace PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(open_telemetry_trace PUBLIC
  client-trace
  opentelemetry-cpp::api
  opentelemetry-cpp::trace
)
_ydb_sdk_make_client_component(OpenTelemetryTrace open_telemetry_trace)

_ydb_sdk_install_headers(${CMAKE_INSTALL_INCLUDEDIR} DIRECTORY include/)