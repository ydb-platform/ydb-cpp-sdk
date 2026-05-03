add_executable(otel_tracing_example)

target_link_libraries(otel_tracing_example PUBLIC
  yutil
  getopt
  YDB-CPP-SDK::Query
  YDB-CPP-SDK::Table
  YDB-CPP-SDK::Params
  YDB-CPP-SDK::Driver
  YDB-CPP-SDK::OpenTelemetryTrace
  YDB-CPP-SDK::OpenTelemetryMetrics
  opentelemetry-cpp::otlp_http_exporter
  opentelemetry-cpp::otlp_http_metric_exporter
)

target_sources(otel_tracing_example PRIVATE
  main.cpp
)

vcs_info(otel_tracing_example)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_link_libraries(otel_tracing_example PUBLIC
    cpuid_check
  )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(otel_tracing_example PRIVATE
    -ldl
    -lrt
    -Wl,--no-as-needed
    -lpthread
  )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_options(otel_tracing_example PRIVATE
    -Wl,-platform_version,macos,11.0,11.0
    -framework
    CoreFoundation
  )
endif()