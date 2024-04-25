add_library(library-grpc-client)

target_link_libraries(library-grpc-client PUBLIC
  yutil
  gRPC::grpc++
  cpp-deprecated-atomic
)

target_sources(library-grpc-client PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/grpc/client/CMakeLists.txt
  grpc_client_low.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/library/grpc/client/grpc_client_low.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/grpc/client/CMakeLists.txt
=======
  grpc_client_low.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/grpc/client/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  grpc_client_low.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  grpc_client_low.cpp
>>>>>>> origin/main
)
