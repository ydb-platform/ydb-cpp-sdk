add_library(cpp-yson-node)

target_link_libraries(cpp-yson-node PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  library-cpp-yson
  cpp-yson-json
)

target_sources(cpp-yson-node PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/yson/node/node.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yson/node/node_io.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yson/node/node_builder.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yson/node/node_visitor.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/yson/node/serialize.cpp
)

generate_enum_serilization(cpp-yson-node
  ${CMAKE_SOURCE_DIR}/library/cpp/yson/node/node.h
  INCLUDE_HEADERS
  library/cpp/yson/node/node.h
)


