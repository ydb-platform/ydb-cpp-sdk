add_library(cpp-openssl-io)

target_link_libraries(cpp-openssl-io PUBLIC
  yutil
  OpenSSL::SSL
  cpp-openssl-init
  cpp-openssl-method
)

target_sources(cpp-openssl-io PRIVATE
<<<<<<< HEAD:src/library/openssl/io/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/io/stream.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/openssl/io/stream.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/openssl/io/CMakeLists.txt
)


