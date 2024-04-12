add_library(monlib-encode-buffered)

target_link_libraries(monlib-encode-buffered PUBLIC
  yutil
  cpp-monlib-encode
  cpp-monlib-metrics
)

target_sources(monlib-encode-buffered PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/buffered_encoder_base.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/buffered/string_pool.cpp
)


