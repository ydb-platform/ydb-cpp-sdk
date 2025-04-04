_ydb_sdk_add_library(client-ydb_topic-include)

target_link_libraries(client-ydb_topic-include
  PUBLIC
    yutil
    api-grpc
    api-protos
    streams-zstd
)

generate_enum_serilization(client-ydb_topic-include
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/control_plane.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/control_plane.h
)

generate_enum_serilization(client-ydb_topic-include
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/read_events.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/read_events.h
)

generate_enum_serilization(client-ydb_topic-include
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/write_events.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/write_events.h
)
