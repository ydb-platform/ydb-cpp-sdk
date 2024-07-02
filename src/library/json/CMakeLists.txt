add_subdirectory(common)
add_subdirectory(fast_sax)
add_subdirectory(writer)

add_library(library-json)

target_link_libraries(library-json PUBLIC
  yutil
  RapidJSON::RapidJSON
  library-json-common
  library-json-fast_sax
  library-json-writer
  string_utils-relaxed_escaper
)

target_sources(library-json PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/json/json_writer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/json/json_reader.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/json/json_prettifier.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/json/rapidjson_helpers.cpp
)

_ydb_sdk_install_targets(TARGETS library-json)
