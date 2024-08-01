if (YDB_SDK_TESTS)
  add_subdirectory(ut)
endif()

add_subdirectory(writer)

add_library(json-common)
target_link_libraries(json-common PUBLIC
  yutil
)
target_sources(json-common PRIVATE
  common/defs.cpp
)
_ydb_sdk_install_targets(TARGETS json-common)


add_library(json-fast_sax)
target_link_libraries(json-fast_sax
  PUBLIC
    yutil
    json-common
)
target_sources(json-fast_sax PRIVATE
  fast_sax/unescape.cpp
)
if (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_ragel_lexers(json-fast_sax
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/library/cpp/json/fast_sax/parser.rl6
    -CT0
  )
else()
  target_ragel_lexers(json-fast_sax
    PRIVATE
    ${YDB_SDK_SOURCE_DIR}/library/cpp/json/fast_sax/parser.rl6
    -CG2
  )
endif()
_ydb_sdk_install_targets(TARGETS json-fast_sax)


add_library(json)
target_link_libraries(json
  PUBLIC
    yutil
    json-common
    json-fast_sax
    json-writer
  PRIVATE
    RapidJSON::RapidJSON
    string_utils-relaxed_escaper
)
target_sources(json
  PRIVATE
    json_writer.cpp
    json_reader.cpp
    json_prettifier.cpp
    rapidjson_helpers.cpp
)
target_compile_definitions(json PUBLIC
  DISABLE_JSON_ESCAPE
)
_ydb_sdk_install_targets(TARGETS json)
