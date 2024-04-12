add_library(cpp-openssl-method)

target_link_libraries(cpp-openssl-method PUBLIC
  yutil
  OpenSSL::SSL
  cpp-openssl-holders
)

target_sources(cpp-openssl-method PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/openssl/method/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/method/io.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/openssl/method/io.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/openssl/method/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/method/io.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
