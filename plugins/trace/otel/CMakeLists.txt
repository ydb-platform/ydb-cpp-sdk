_ydb_sdk_add_library(open_telemetry_trace)
target_sources(open_telemetry_trace PRIVATE
  trace.cpp
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

_ydb_sdk_directory_install(FILES ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/open_telemetry/trace.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ydb-cpp-sdk/open_telemetry COMPONENT libydb-cpp-otel-tracing)