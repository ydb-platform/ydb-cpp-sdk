add_subdirectory(protos)

add_library(library-operation_id)

target_link_libraries(library-operation_id PUBLIC
  yutil
  protobuf::libprotobuf
  library-cpp-cgiparam
  library-cpp-uri
  lib-operation_id-protos
)

target_sources(library-operation_id PRIVATE
  operation_id.cpp
)
