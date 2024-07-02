add_library(library-json-writer)

target_link_libraries(library-json-writer PUBLIC
  yutil
  library-json-common
  enum_serialization_runtime
  string_utils-misc
)

target_sources(library-json-writer PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/json/writer/json_value.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/json/writer/json.cpp
)

generate_enum_serilization(library-json-writer
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/json/writer/json_value.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/json/writer/json_value.h
)

_ydb_sdk_install_targets(TARGETS library-json-writer)
