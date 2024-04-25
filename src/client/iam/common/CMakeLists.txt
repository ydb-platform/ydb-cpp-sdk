add_library(client-iam-common)

target_link_libraries(client-iam-common PUBLIC
  yutil
  library-grpc-client
  cpp-http-simple
  library-cpp-json
  cpp-threading-atomic
  library-jwt
  client-ydb_types-credentials
)

target_sources(client-iam-common PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/iam/common/CMakeLists.txt
  iam.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/iam/common/iam.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/iam/common/CMakeLists.txt
=======
  iam.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/iam/common/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  iam.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  iam.cpp
>>>>>>> origin/main
)


