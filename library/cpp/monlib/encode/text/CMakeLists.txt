add_library(monlib-encode-text)

target_link_libraries(monlib-encode-text PUBLIC
  yutil
  cpp-monlib-encode
)

target_sources(monlib-encode-text PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/text/text_encoder.cpp
)
