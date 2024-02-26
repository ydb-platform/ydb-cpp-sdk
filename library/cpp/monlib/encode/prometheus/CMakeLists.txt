add_library(monlib-encode-prometheus)

target_link_libraries(monlib-encode-prometheus PUBLIC
  contrib-libs-cxxsupp
  yutil
  cpp-monlib-encode
  monlib-encode-buffered
  cpp-string_builder
)

target_sources(monlib-encode-prometheus PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/prometheus/prometheus_decoder.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/encode/prometheus/prometheus_encoder.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(monlib-encode-prometheus PUBLIC
    contrib-libs-linux-headers
  )
endif()
