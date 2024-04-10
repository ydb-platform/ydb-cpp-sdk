add_library(cpp-threading-chunk_queue)

target_link_libraries(cpp-threading-chunk_queue PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-chunk_queue PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/chunk_queue/queue.cpp
)
