get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

add_library(cpp-json-writer)

target_link_libraries(cpp-json-writer PUBLIC
  contrib-libs-linux-headers
  contrib-libs-cxxsupp
  yutil
  cpp-json-common
  tools-enum_parser-enum_serialization_runtime
  cpp-string_builder
  cpp-string_utils-misc
)

target_sources(cpp-json-writer PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/json/writer/json_value.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/json/writer/json.cpp
)

generate_enum_serilization(cpp-json-writer
  ${CMAKE_SOURCE_DIR}/library/cpp/json/writer/json_value.h
  INCLUDE_HEADERS
  library/cpp/json/writer/json_value.h
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(cpp-json-writer PUBLIC
    contrib-libs-linux-headers
  )
endif()
