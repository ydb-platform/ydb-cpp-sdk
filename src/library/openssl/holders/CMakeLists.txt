add_library(cpp-openssl-holders)

target_link_libraries(cpp-openssl-holders PUBLIC
  yutil
  OpenSSL::SSL
)

target_sources(cpp-openssl-holders PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/openssl/holders/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/holders/bio.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/holders/x509_vfy.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/openssl/holders/bio.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/openssl/holders/x509_vfy.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/openssl/holders/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/holders/bio.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/holders/x509_vfy.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/openssl/holders/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)
