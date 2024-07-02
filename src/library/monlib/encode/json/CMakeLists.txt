add_library(monlib-encode-json)

target_link_libraries(monlib-encode-json PUBLIC
  yutil
  monlib-encode
  monlib-encode-buffered
  monlib-exception
  library-json
  library-json-writer
)

target_sources(monlib-encode-json PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/json/json_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/json/json_encoder.cpp
)

_ydb_sdk_install_targets(TARGETS monlib-encode-json)
