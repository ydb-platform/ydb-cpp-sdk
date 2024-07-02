add_library(monlib-encode-text)

target_link_libraries(monlib-encode-text PUBLIC
  yutil
  monlib-encode
)

target_sources(monlib-encode-text PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/text/text_encoder.cpp
)

_ydb_sdk_install_targets(TARGETS monlib-encode-text)
