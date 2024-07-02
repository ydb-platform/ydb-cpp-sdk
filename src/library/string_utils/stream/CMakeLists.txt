add_library(string_utils-stream)

target_link_libraries(string_utils-stream PUBLIC
  yutil
)

target_sources(string_utils-stream PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/stream/stream.cpp
)

_ydb_sdk_install_targets(TARGETS string_utils-stream)
