add_library(library-http-server)

target_link_libraries(library-http-server PUBLIC
  yutil
  library-http-misc
  library-http-io
  threading-equeue
  deprecated-atomic
)

target_sources(library-http-server PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/http/server/conn.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/server/http.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/server/http_ex.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/server/options.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/http/server/response.cpp
)

_ydb_sdk_install_targets(TARGETS library-http-server)
