add_library(cpp-http-fetch)

target_link_libraries(cpp-http-fetch PUBLIC
  yutil
  ZLIB::ZLIB
  library-cpp-charset
  cpp-digest-md5
  cpp-http-misc
  library-cpp-logger
  cpp-mime-types
  library-cpp-uri
  tools-enum_parser-enum_serialization_runtime
)

target_sources(cpp-http-fetch PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/http_digest.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/http_socket.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/httpheader.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/httpload.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/exthttpcodes.cpp
)

generate_enum_serilization(cpp-http-fetch
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/httpheader.h
  INCLUDE_HEADERS
  src/library/http/fetch/httpheader.h
)

target_ragel_lexers(cpp-http-fetch
  PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/httpfsm.rl6
  -CG1
)
