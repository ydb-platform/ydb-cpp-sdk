add_library(cpp-openssl-init)

target_link_libraries(cpp-openssl-init PUBLIC
  yutil
  OpenSSL::SSL
)

target_sources(cpp-openssl-init PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/openssl/init/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/init/init.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/openssl/init/init.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/openssl/init/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/init/init.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/openssl/init/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/openssl/init/init.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
