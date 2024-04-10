add_library(cpp-http-server)

target_link_libraries(cpp-http-server PUBLIC
  yutil
  cpp-http-misc
  cpp-http-io
  cpp-threading-equeue
  cpp-deprecated-atomic
)

target_sources(cpp-http-server PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/server/conn.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/server/http.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/server/http_ex.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/server/options.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/server/response.cpp
)


