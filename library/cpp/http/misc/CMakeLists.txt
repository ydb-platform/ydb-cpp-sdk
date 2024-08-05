_ydb_sdk_add_library(http-misc)

target_link_libraries(http-misc
  PUBLIC
    yutil
    cgiparam
    digest-lower_case
  PRIVATE
    enum_serialization_runtime
    case_insensitive_string
)

target_sources(http-misc
  PRIVATE
    httpcodes.cpp
    httpdate.cpp
    httpreqdata.cpp
    parsed_request.cpp
)

generate_enum_serilization(http-misc
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/misc/httpcodes.h
  INCLUDE_HEADERS
  library/cpp/http/misc/httpcodes.h
)

_ydb_sdk_install_targets(TARGETS http-misc)
