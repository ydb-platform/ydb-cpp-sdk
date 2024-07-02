add_library(library-yson-json)

target_link_libraries(library-yson-json PUBLIC
  yutil
  library-json
)

target_sources(library-yson-json PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/json/json_writer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/json/yson2json_adapter.cpp
)

_ydb_sdk_install_targets(TARGETS library-yson-json)
