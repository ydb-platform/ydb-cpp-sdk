add_library(proto_output)

target_link_libraries(proto_output PUBLIC
  yutil
  api-protos
  protobuf::libprotobuf
)

target_sources(proto_output PRIVATE
  proto_output.cpp
)

