_ydb_sdk_add_library(http-server)

target_link_libraries(http-server
  PUBLIC
    yutil
    http-misc
    http-io
  PRIVATE
    threading-equeue
)

target_sources(http-server
  PRIVATE
    conn.cpp
    http.cpp
    http_ex.cpp
    options.cpp
    response.cpp
)

_ydb_sdk_install_targets(TARGETS http-server)
