add_library(http-fetch)

target_link_libraries(http-fetch
  PUBLIC
    yutil
    http-misc
    logger
    mime-types
    library-uri
    ZLIB::ZLIB
  PRIVATE
    charset
    digest-md5
    enum_serialization_runtime
)

target_sources(http-fetch PRIVATE
  http_digest.cpp
  http_socket.cpp
  httpheader.cpp
  httpload.cpp
  exthttpcodes.cpp
)

generate_enum_serilization(http-fetch
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/fetch/httpheader.h
  INCLUDE_HEADERS
  library/cpp/http/fetch/httpheader.h
)

target_ragel_lexers(http-fetch
  PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/fetch/httpfsm.rl6
  -CG1
)

_ydb_sdk_install_targets(TARGETS http-fetch)
