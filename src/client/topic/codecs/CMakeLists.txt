add_library(client-ydb_topic-codecs)

target_link_libraries(client-ydb_topic-codecs PUBLIC
  yutil
  cpp-streams-zstd
  public-issue-protos
  api-grpc-draft
  api-grpc
  api-protos
)

target_sources(client-ydb_topic-codecs PRIVATE
  codecs.cpp
)

generate_enum_serilization(client-ydb_topic-codecs
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/codecs/codecs.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/codecs/codecs.h
)
