add_library(cpp-yson-node)

target_link_libraries(cpp-yson-node PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  library-cpp-yson
  cpp-yson-json
)

target_sources(cpp-yson-node PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/node/node.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/node/node_io.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/node/node_builder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/node/node_visitor.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/node/serialize.cpp
)

generate_enum_serilization(cpp-yson-node
  ${YDB_SDK_SOURCE_DIR}/src/library/yson/node/node.h
  INCLUDE_HEADERS
  src/library/yson/node/node.h
)


