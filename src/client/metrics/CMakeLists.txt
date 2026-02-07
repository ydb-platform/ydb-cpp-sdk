add_library(client-metrics
    metrics.cpp
    otel/otel.cpp
)

target_include_directories(client-metrics PUBLIC
    $<BUILD_INTERFACE:${YDB_SDK_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(client-metrics PUBLIC
    opentelemetry-cpp::api
    opentelemetry-cpp::metrics
    opentelemetry-cpp::trace
)
