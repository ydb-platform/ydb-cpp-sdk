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
  ${CMAKE_SOURCE_DIR}/client/ydb_topic/codecs/codecs.cpp
)


