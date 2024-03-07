add_library(client-ydb_persqueue_public-codecs)

target_link_libraries(client-ydb_persqueue_public-codecs PUBLIC
  contrib-libs-linux-headers
  contrib-libs-cxxsupp
  yutil
  cpp-streams-zstd
  public-issue-protos
  api-grpc-draft
  api-grpc
  api-protos
  )

target_sources(client-ydb_persqueue_public-codecs PRIVATE
  ${CMAKE_SOURCE_DIR}/client/ydb_persqueue_public/codecs/codecs.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(client-ydb_persqueue_public-codecs PUBLIC
    contrib-libs-linux-headers
  )
endif()
