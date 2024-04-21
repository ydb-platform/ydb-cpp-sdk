add_library(cpp-yson-node)

target_link_libraries(cpp-yson-node PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  library-cpp-yson
  cpp-yson-json
)

target_sources(cpp-yson-node PRIVATE
  node.cpp
  node_io.cpp
  node_builder.cpp
  node_visitor.cpp
  serialize.cpp
)

generate_enum_serilization(cpp-yson-node
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/yson/node/node.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/yson/node/node.h
)


