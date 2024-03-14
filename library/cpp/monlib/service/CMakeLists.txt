add_subdirectory(pages)

add_library(cpp-monlib-service)

target_link_libraries(cpp-monlib-service PUBLIC
  yutil
  cpp-string_utils-base64
  protobuf::libprotobuf
  cpp-coroutine-engine
  cpp-coroutine-listener
  cpp-http-fetch
  cpp-http-server
  cpp-http-io
  library-cpp-logger
  cpp-malloc-api
  library-cpp-svnversion
  library-cpp-uri
  library-cpp-cgiparam
)

target_sources(cpp-monlib-service PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/monservice.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/mon_service_http_request.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/service.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/format.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/auth.cpp
)



