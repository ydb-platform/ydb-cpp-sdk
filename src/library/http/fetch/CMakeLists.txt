add_library(library-http-fetch)

target_link_libraries(library-http-fetch PUBLIC
  yutil
  ZLIB::ZLIB
  library-charset
  digest-md5
  library-http-misc
  library-logger
  library-mime-types
  library-uri
  enum_serialization_runtime
)

target_sources(library-http-fetch PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/http_digest.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/http_socket.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/httpheader.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/httpload.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/exthttpcodes.cpp
)

generate_enum_serilization(library-http-fetch
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/http/fetch/httpheader.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/http/fetch/httpheader.h
)

target_ragel_lexers(library-http-fetch
  PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/httpfsm.rl6
  -CG1
)

_ydb_sdk_install_targets(TARGETS library-http-fetch)
