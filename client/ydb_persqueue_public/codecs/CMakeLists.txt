add_library(client-ydb_persqueue_public-codecs)

target_link_libraries(client-ydb_persqueue_public-codecs PUBLIC
  
  yutil
  cpp-streams-zstd
  public-issue-protos
  api-grpc-draft
  api-grpc
  api-protos
  )

target_sources(client-ydb_persqueue_public-codecs PRIVATE
  ${YDB_SDK_SOURCE_DIR}/client/ydb_persqueue_public/codecs/codecs.cpp
)


