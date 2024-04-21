add_library(cpp-threading-future)

target_link_libraries(cpp-threading-future PUBLIC
  yutil
)

target_sources(cpp-threading-future PRIVATE
  async_semaphore.cpp
  async.cpp
  core/future.cpp
  wait/wait.cpp
)
