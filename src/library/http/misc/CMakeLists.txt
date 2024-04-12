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
  ${YDB_SDK_SOURCE_DIR}/src/library/http/misc/httpcodes.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/misc/httpdate.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/misc/httpreqdata.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/misc/parsed_request.cpp
)

generate_enum_serilization(cpp-http-misc
  ${YDB_SDK_SOURCE_DIR}/src/library/http/misc/httpcodes.h
  INCLUDE_HEADERS
  src/library/http/misc/httpcodes.h
)


