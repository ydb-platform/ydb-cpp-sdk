add_library(library-grpc-client)

target_link_libraries(library-grpc-client PUBLIC
  yutil
  gRPC::grpc++
  cpp-deprecated-atomic
)

target_sources(library-grpc-client PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/library/grpc/client/grpc_client_low.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-grpc-client PUBLIC
    
  )
endif()
