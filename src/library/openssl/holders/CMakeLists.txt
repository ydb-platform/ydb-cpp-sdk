add_library(cpp-openssl-holders)

target_link_libraries(cpp-openssl-holders PUBLIC
  yutil
  OpenSSL::SSL
)

target_sources(cpp-openssl-holders PRIVATE
<<<<<<< HEAD:src/library/openssl/holders/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/holders/bio.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/holders/x509_vfy.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/openssl/holders/bio.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/openssl/holders/x509_vfy.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/openssl/holders/CMakeLists.txt
)
