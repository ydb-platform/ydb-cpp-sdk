add_library(library-grpc-client)

target_link_libraries(library-grpc-client PUBLIC
  contrib-libs-cxxsupp
  yutil
  gRPC::grpc++
  cpp-deprecated-atomic
  cpp-string_builder
)

target_sources(library-grpc-client PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/library/grpc/client/grpc_client_low.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-grpc-client PUBLIC
    contrib-libs-linux-headers
  )
endif()
