add_library(library-yson-node)

target_link_libraries(library-yson-node PUBLIC
  yutil
  enum_serialization_runtime
  library-yson
  library-yson-json
)

target_sources(library-yson-node PRIVATE
  node.cpp
  node_io.cpp
  node_builder.cpp
  node_visitor.cpp
  serialize.cpp
)

generate_enum_serilization(library-yson-node
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/yson/node/node.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/yson/node/node.h
)

_ydb_sdk_install_targets(TARGETS library-yson-node)
