add_subdirectory(pages)

add_library(monlib-service)

target_link_libraries(monlib-service PUBLIC
  yutil
  string_utils-base64
  protobuf::libprotobuf
  library-coroutine-engine
  library-coroutine-listener
  library-http-fetch
  library-http-server
  library-http-io
  library-logger
  library-svnversion
  library-uri
  library-cgiparam
)

target_sources(monlib-service PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/monservice.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/mon_service_http_request.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/service.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/format.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/auth.cpp
)

_ydb_sdk_install_targets(TARGETS monlib-service)
