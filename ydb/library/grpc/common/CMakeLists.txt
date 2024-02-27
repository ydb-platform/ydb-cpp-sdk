add_library(library-grpc-common INTERFACE)

target_link_libraries(library-grpc-common INTERFACE
  contrib-libs-cxxsupp
  yutil
  gRPC::grpc++
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux"  AND NOT HAVE_CUDA)
  target_link_libraries(library-grpc-common INTERFACE
    contrib-libs-linux-headers
  )
endif()
