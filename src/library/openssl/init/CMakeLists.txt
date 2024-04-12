add_library(cpp-openssl-init)

target_link_libraries(cpp-openssl-init PUBLIC
  yutil
  OpenSSL::SSL
)

target_sources(cpp-openssl-init PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/openssl/init/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/init/init.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/openssl/init/init.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/openssl/init/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/init/init.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
