add_library(library-http-misc)

target_link_libraries(library-http-misc PUBLIC
  yutil
  enum_serialization_runtime
  library-case_insensitive_string
  library-cgiparam
  digest-lower_case
  string_utils-misc
)

target_sources(library-http-misc PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/http/misc/httpcodes.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/misc/httpdate.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/misc/httpreqdata.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/misc/parsed_request.cpp
)

generate_enum_serilization(library-http-misc
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/http/misc/httpcodes.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/http/misc/httpcodes.h
)

_ydb_sdk_install_targets(TARGETS library-http-misc)
