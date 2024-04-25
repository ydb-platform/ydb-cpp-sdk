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
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/http/fetch/httpheader.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/http/fetch/httpheader.h
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/httpheader.h
  INCLUDE_HEADERS
  src/library/http/fetch/httpheader.h
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/http/fetch/httpheader.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/http/fetch/httpheader.h
>>>>>>> origin/main
)

target_ragel_lexers(cpp-http-fetch
  PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/http/fetch/httpfsm.rl6
  -CG1
)
