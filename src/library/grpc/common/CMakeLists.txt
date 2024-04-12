add_library(library-grpc-common INTERFACE)

target_link_libraries(library-grpc-common INTERFACE
  yutil
  gRPC::grpc++
)
