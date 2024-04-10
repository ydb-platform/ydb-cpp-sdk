add_library(cpp-http-simple)

target_link_libraries(cpp-http-simple PUBLIC
  yutil
  cpp-http-io
  cpp-openssl-io
  cpp-string_utils-url
  library-cpp-uri
)

target_sources(cpp-http-simple PRIVATE
<<<<<<< HEAD:src/library/http/simple/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/http/simple/http_client.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/http/simple/http_client.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/http/simple/CMakeLists.txt
)


