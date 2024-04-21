add_library(cpp-http-simple)

target_link_libraries(cpp-http-simple PUBLIC
  yutil
  cpp-http-io
  cpp-openssl-io
  cpp-string_utils-url
  library-cpp-uri
)

target_sources(cpp-http-simple PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/http/simple/http_client.cpp
)


