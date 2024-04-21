add_library(library-grpc-client)

target_link_libraries(library-grpc-client PUBLIC
  yutil
  gRPC::grpc++
  cpp-deprecated-atomic
)

target_sources(library-grpc-client PRIVATE
  grpc_client_low.cpp
)
