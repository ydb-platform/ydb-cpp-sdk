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
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/http/misc/httpcodes.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/http/misc/httpcodes.h
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/http/misc/httpcodes.h
  INCLUDE_HEADERS
  src/library/http/misc/httpcodes.h
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)


