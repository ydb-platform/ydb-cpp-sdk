add_library(library-grpc-client)

target_link_libraries(library-grpc-client PUBLIC
  yutil
  gRPC::grpc++
  cpp-deprecated-atomic
)

target_sources(library-grpc-client PRIVATE
<<<<<<< HEAD:src/library/grpc/client/CMakeLists.txt
  grpc_client_low.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/library/grpc/client/grpc_client_low.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/grpc/client/CMakeLists.txt
)
