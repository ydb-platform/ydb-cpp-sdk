get_built_tool_path(
  TOOL_enum_parser_bin
  TOOL_enum_parser_dependency
  tools/enum_parser/enum_parser
  enum_parser
)

add_library(cpp-http-misc)

target_link_libraries(cpp-http-misc PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  library-cpp-case_insensitive_string
  library-cpp-cgiparam
  cpp-digest-lower_case
  cpp-string_utils-misc
)

target_sources(cpp-http-misc PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/http/misc/httpcodes.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/http/misc/httpdate.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/http/misc/httpreqdata.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/http/misc/parsed_request.cpp
)

generate_enum_serilization(cpp-http-misc
  ${CMAKE_SOURCE_DIR}/library/cpp/http/misc/httpcodes.h
  INCLUDE_HEADERS
  library/cpp/http/misc/httpcodes.h
)


