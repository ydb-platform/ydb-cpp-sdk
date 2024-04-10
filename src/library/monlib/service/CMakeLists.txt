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
  library-cpp-svnversion
  library-cpp-uri
  library-cpp-cgiparam
)

target_sources(cpp-monlib-service PRIVATE
<<<<<<< HEAD:src/library/monlib/service/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/monservice.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/mon_service_http_request.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/service.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/format.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/auth.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/service/monservice.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/service/mon_service_http_request.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/service/service.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/service/format.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/service/auth.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/service/CMakeLists.txt
)



